#include <QApplication>
#include <QLockFile>
#include "config.h"

int main(int argc, char *argv[])
{
    QString lockFileName("app.lock");
    QFile file( lockFileName );

    if ( file.exists() ) {
        return -1;
    }

    QLockFile lockFile( lockFileName );
    if ( !lockFile.isLocked() ) {

        QApplication a(argc, argv);
        MainWindow w;
        Player* player;

        lockFile.lock();

        w.show();
        if ( argc < 2 ) {
            player = new Player();
        }else {
            QString s(argv[1]);
            QStringList list(s);
            player = new Player(list);
        }
        w.init_lockFile(&lockFile);
        w.init_player(player);

        // 歌词文件夹不存在就创建
        QDir dir;
        if ( !dir.exists( QString("./lyrics") ) ) {
            dir.mkpath(QString("./lyrics"));
        }
        return a.exec();
    }else {
        return -1;
    }
}
