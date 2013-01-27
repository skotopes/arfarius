#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>

class MacSupport;
class MainWindow;
class Player;
class PlayListModel;

class Application : public QApplication
{
    friend int main(int,char*[]);
    Q_OBJECT

public:
    Application(int, char*[]);
    ~Application();

private:
    MacSupport *platform_support;
    MainWindow *main_window;
    Player *player;
    PlayListModel *playlist;
};

#endif // APPLICATION_H
