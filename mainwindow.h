#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMenu>
#include <QWidget>
#include <QLayout>
#include <QMenuBar>
#include <QSplitter>
#include <QTreeView>
#include <QStandardItemModel>

#include "rightwidget.h"
#include "player.h"
#include "toast.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
public:
    void init_player(Player* player);
    void init_lockFile(QLockFile* file);
    Toast *m_toast;
private:
    void    init_widget(void);
    void    init_menu(void);
    void    init_leftWidget(void);
    void    init_rightWidget(void);
    void    init_contextMenu(void);
    void    timerEvent(QTimerEvent *event);
    void    removeItem(int index);
    void    saveData(void);
private:
    QVBoxLayout *m_mainLayout;
    QSplitter   *m_mainWidget;
    QMenuBar    *m_menuBar;
    QMenu       *m_fileMenu;
    QMenu       *m_helpMenu;
    QMenu       *m_contextMenu;
    QAction     *m_addMusic;
    QAction     *m_exit;
    QAction     *m_about;
    QTreeView   *m_musicListWidget;
    QStandardItemModel *m_musicListModel;

    RightWidget *m_rightWidget;
private:
    Player          *_m_player;
    QLockFile       *_m_lockFile;
    int             _m_removeId;

    static QString  sm_userDb;
    const std::vector<std::pair<int, QString> >* _m_lyricsList;
private slots:
    void    AddMusic();
    void    About();
    void    PlayMusic(const QModelIndex &);
    void    Play(bool custom = false);
    void    Next();
    void    Prev();
    void    positionChange(qint64 pos);
    void    GetMetaInfo();
    void    ChangePositionCustom();
    void    contextMenu(QPoint p);
    void    remove(bool r);
};

void ShowToast( MainWindow* obj, const QString msg );

#endif // MAINWINDOW_H
