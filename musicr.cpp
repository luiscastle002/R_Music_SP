#include <iostream>
#include <memory>
#include <string>
#include <vector>
using namespace std;
struct Song {
    string title, artist, format;
};
vector<Song> songs = {
    {"Blinding Lights", "The Weeknd", "MP3"},
    {"Bohemian Rhapsody", "Queen", "FLAC"},
    {"Viva La Vida", "Coldplay", "MP3"}
};
// PATRON 1: FACTORY METHOD
class IPlayer {
public:
    virtual ~IPlayer() = default;
    virtual void play(string song) = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
};
// PATRON 2: ADAPTER
class Mp3Engine {
public:
    void start(string s) { cout << "[MP3] Reproduciendo: " << s << "\n"; }
    void suspend() { cout << "[MP3] Pausado\n"; }
    void end() { cout << "[MP3] Detenido\n"; }
};
class FlacEngine {
public:
    void decode(string s) { cout << "[FLAC] Reproduciendo: " << s << "\n"; }
    void suspendPlayback() { cout << "[FLAC] Pausado\n"; }
    void closeFile() { cout << "[FLAC] Detenido\n"; }
};
class Mp3Adapter : public IPlayer {
    Mp3Engine engine;
public:
    void play(string s) override { engine.start(s); }
    void pause() override { engine.suspend(); }
    void stop() override { engine.end(); }
};
class FlacAdapter : public IPlayer {
    FlacEngine engine;
public:
    void play(string s) override { engine.decode(s); }
    void pause() override { engine.suspendPlayback(); }
    void stop() override { engine.closeFile(); }
};
// FACTORY METHOD: cada creador produce un adaptador
class PlayerCreator {
public:
    virtual ~PlayerCreator() = default;
    virtual unique_ptr<IPlayer> create() = 0;
};
class Mp3Creator : public PlayerCreator {
public:
    unique_ptr<IPlayer> create() override {
        return make_unique<Mp3Adapter>();
    }
};
class FlacCreator : public PlayerCreator {
public:
    unique_ptr<IPlayer> create() override {
        return make_unique<FlacAdapter>();
    }
};
// PATRON 3: STATE
class MusicPlayer;
class State {
public:
    virtual ~State() = default;
    virtual void play(MusicPlayer&) = 0;
    virtual void pause(MusicPlayer&) = 0;
    virtual void stop(MusicPlayer&) = 0;
    virtual string name() = 0;
};
class Stopped : public State {
public:
    void play(MusicPlayer&) override;
    void pause(MusicPlayer&) override;
    void stop(MusicPlayer&) override;
    string name() override { return "DETENIDO"; }
};
class Playing : public State {
public:
    void play(MusicPlayer&) override;
    void pause(MusicPlayer&) override;
    void stop(MusicPlayer&) override;
    string name() override { return "REPRODUCIENDO"; }
};
class Paused : public State {
public:
    void play(MusicPlayer&) override;
    void pause(MusicPlayer&) override;
    void stop(MusicPlayer&) override;
    string name() override { return "PAUSADO"; }
};
class MusicPlayer {
    unique_ptr<IPlayer> device;
    unique_ptr<State> state;
    string song;
public:
    MusicPlayer(unique_ptr<IPlayer> p, string s)
        : device(move(p)), state(make_unique<Stopped>()), song(s) {}

    void change(unique_ptr<State> s) { state = move(s); }
    void playDevice() { device->play(song); }
    void pauseDevice() { device->pause(); }
    void stopDevice() { device->stop(); }
    void play() { state->play(*this); }
    void pause() { state->pause(*this); }
    void stop() { state->stop(*this); }
    string status() { return state->name(); }
};
void Stopped::play(MusicPlayer& p) {
    p.playDevice();
    p.change(make_unique<Playing>());
}
void Stopped::pause(MusicPlayer&) {
    cout << "No puedes pausar: esta detenido.\n";
}
void Stopped::stop(MusicPlayer&) {
    cout << "Ya esta detenido.\n";
}
void Playing::play(MusicPlayer&) {
    cout << "La cancion ya esta reproduciendose.\n";
}
void Playing::pause(MusicPlayer& p) {
    p.pauseDevice();
    p.change(make_unique<Paused>());
}
void Playing::stop(MusicPlayer& p) {
    p.stopDevice();
    p.change(make_unique<Stopped>());
}
void Paused::play(MusicPlayer& p) {
    p.playDevice();
    p.change(make_unique<Playing>());
}
void Paused::pause(MusicPlayer&) {
    cout << "La cancion ya esta pausada.\n";
}
void Paused::stop(MusicPlayer& p) {
    p.stopDevice();
    p.change(make_unique<Stopped>());
}
// FLUJO PRINCIPAL: INTEGRACION DE LOS TRES PATRONES
int main() {
    cout << "=== MINIPLAYER ===\n";
    for (int i = 0; i < (int)songs.size(); i++)
        cout << i + 1 << ". " << songs[i].title << " - "
             << songs[i].artist << " [" << songs[i].format << "]\n";

    cout << "Selecciona una cancion (0 para salir): ";
    int n;
    if (!(cin >> n) || n < 0 || n > (int)songs.size()) {
        cout << "Seleccion invalida.\n";
        return 1;
    }
    if (n == 0) return 0;

    Song selected = songs[n - 1];

    // Factory Method selecciona el creador; Adapter unifica la interfaz
    unique_ptr<PlayerCreator> creator;
    if (selected.format == "MP3")
        creator = make_unique<Mp3Creator>();
    else
        creator = make_unique<FlacCreator>();

    auto device = creator->create();
    cout << "Formato: " << selected.format << "\n";

    // State controla las acciones permitidas
    MusicPlayer player(move(device), selected.title);
    int op = -1;
    while (op != 0) {
        cout << "\nEstado: " << player.status()
             << "\n1. Reproducir/reanudar\n2. Pausar"
             << "\n3. Detener\n0. Salir\nOpcion: ";
        if (!(cin >> op)) break;

        switch (op) {
            case 1: player.play(); break;
            case 2: player.pause(); break;
            case 3: player.stop(); break;
            case 0: cout << "Cerrando MiniPlayer...\n"; break;
            default: cout << "Opcion no valida.\n";
        }
    }
    return 0;
}