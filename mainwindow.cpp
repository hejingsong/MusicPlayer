#include <QLabel>
#include <QScrollArea>
#include <QMessageBox>
#include <QFileDialog>
#include <QMediaObject>
#include "config.h"

QString MainWindow::sm_userDb = "user.db";

void ShowToast( MainWindow* obj, const QString msg ) {
    obj->m_toast = new Toast(msg, obj);
    obj->m_toast->show();
    obj->startTimer(2000);
}

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    this->_m_player = nullptr;
    this->_m_lyricsList = nullptr;
    this->_m_lockFile = nullptr;
    this->_m_removeId = -1;
    this->init_widget();
}

MainWindow::~MainWindow()
{
#if Debug
    qDebug() << "app exit";
#endif
    this->saveData();
    this->_m_lockFile->unlock();
    delete this->_m_player;
}

void MainWindow::init_widget(void) {
    this->setWindowTitle(tr("MusicPlayer"));
    this->setMinimumSize(1000, 600);

    this->m_mainLayout = new QVBoxLayout;

    this->init_menu();
    this->m_mainWidget = new QSplitter(Qt::Horizontal, this);
    this->m_mainLayout->addWidget(this->m_mainWidget);
    this->m_mainLayout->setContentsMargins(0, 0, 0, 0);
    this->m_mainLayout->setSpacing(0);
    this->setLayout(this->m_mainLayout);

    this->init_leftWidget();
    this->init_rightWidget();
    this->init_contextMenu();

    this->m_mainWidget->setStretchFactor(0, 20);
    this->m_mainWidget->setStretchFactor(1, 80);
    this->m_mainWidget->setAutoFillBackground(true);
}

void MainWindow::init_lockFile(QLockFile* file) {
    this->_m_lockFile = file;
}

void MainWindow::init_menu(void) {
    this->m_menuBar = new QMenuBar(this);
    this->m_fileMenu = this->m_menuBar->addMenu(tr("File"));
    this->m_helpMenu = this->m_menuBar->addMenu(tr("Help"));

    this->m_addMusic = new QAction(tr("Add music"), this);
    this->m_exit = new QAction(tr("Exit"), this);
    this->m_about = new QAction(tr("About"), this);

    this->m_fileMenu->addAction(this->m_addMusic);
    this->m_fileMenu->addSeparator();
    this->m_fileMenu->addAction(this->m_exit);

    this->m_helpMenu->addAction(this->m_about);

    this->m_menuBar->setFixedHeight(this->m_menuBar->sizeHint().height());
    this->m_mainLayout->addWidget(this->m_menuBar);

    connect(this->m_addMusic, SIGNAL(triggered(bool)), this, SLOT(AddMusic()));
    connect(this->m_exit, SIGNAL(triggered(bool)), this, SLOT(close()));
    connect(this->m_about, SIGNAL(triggered(bool)), this, SLOT(About()));
}

void MainWindow::init_leftWidget(void) {
    this->m_musicListWidget = new QTreeView(this);
    this->m_musicListModel = new QStandardItemModel(this);
    this->m_musicListModel->setHorizontalHeaderLabels(QStringList("Music List"));
    this->m_musicListWidget->setModel(this->m_musicListModel);
    this->m_musicListWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->m_mainWidget->addWidget(this->m_musicListWidget);

    this->m_musicListWidget->setStyleSheet("QTreeView{ background-color: #58677A; font-size: 15px; color: white; border: none; }"
                                           "QTreeView::branch{ background:#5B677A; }"
                                           "QTreeView::item:hover { background: rgb(69, 187, 217); }"
                                           "QTreeView::item{ margin: 2px; background: #5B677A; background-clip: margin; }"
                                           "QTreeView::item:selected:active{ background: rgb(63, 147, 168); }"
                                           "QTreeView::item:selected:!active{ background: rgb(63, 147, 168); }");

    this->m_musicListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this->m_musicListWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    connect(this->m_musicListWidget, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(PlayMusic(const QModelIndex &)));
}

void MainWindow::init_rightWidget(void) {
    this->m_rightWidget = new RightWidget(this);
    this->m_mainWidget->addWidget(this->m_rightWidget);

    connect( this->m_rightWidget->m_stopBtn, SIGNAL(clicked(bool)), this, SLOT(Play()));
    connect( this->m_rightWidget->m_nextBtn, SIGNAL(clicked(bool)), this, SLOT(Next()));
    connect( this->m_rightWidget->m_prevBtn, SIGNAL(clicked(bool)), this, SLOT(Prev()));
    connect( this->m_rightWidget->m_processBar, SIGNAL(sliderReleased()), this, SLOT(ChangePositionCustom()));
}

void MainWindow::init_contextMenu(void) {
    this->m_contextMenu = new QMenu(this);
    QAction *ac = new QAction(tr("删除"), this);

    this->m_contextMenu->addAction(ac);
    connect(ac, SIGNAL(triggered(bool)), this, SLOT(remove(bool)));
}

void MainWindow::init_player(Player* player) {
    this->_m_player = player;
    this->_m_lyricsList = player->GetLyricsList();
    connect(player->GetPlayer(), SIGNAL(positionChanged(qint64)), this, SLOT(positionChange(qint64)));
    connect(player->GetPlayer(), SIGNAL(metaDataChanged()), this, SLOT(GetMetaInfo()));

    QFile file( MainWindow::sm_userDb );
    QStringList fileList;
    if ( !file.exists() ) {
        return;
    }
    file.open(QFile::ReadOnly);
    while( !file.atEnd() ) {
        QString buf = file.readLine(1024);
        buf.replace(tr("\n"), tr(""));
        fileList.append(buf);
    }
    file.close();
    this->_m_player->append(fileList);
    foreach ( QString f, fileList ) {
        QStringList tmp = f.split("/");
        QStandardItem* item = new QStandardItem(QIcon(":/resource/music.ico"), tmp.back());
        this->m_musicListModel->appendRow(item);
    }
}

void MainWindow::saveData(void) {
    QMediaPlaylist* list = this->_m_player->GetPlayer()->playlist();
    int size = list->mediaCount();
    QFile obj( MainWindow::sm_userDb );
    obj.open(QFile::WriteOnly);

    for (int i=0; i<size; i++) {
        QString file = list->media(i).canonicalUrl().toString();
        file.append('\n');
        obj.write(file.toUtf8());
    }
    obj.close();
}

void MainWindow::removeItem(int index) {
    this->m_musicListModel->removeRow(index);
}

void MainWindow::remove(bool) {
    this->m_musicListModel->removeRow(this->_m_removeId);
    this->_m_player->remove(this->_m_removeId);
}

void MainWindow::contextMenu(QPoint p) {
    QModelIndex index = this->m_musicListWidget->indexAt(p);
    int row = index.row();
    if ( row == -1 ) {
        return;
    }
    this->_m_removeId = row;
    this->m_contextMenu->exec(QCursor::pos());
}

void MainWindow::AddMusic() {
    QStringList musics = QFileDialog::getOpenFileNames(this, tr("musics"),
                                  tr("."),
                                  tr("*.mp3"));
    if ( musics.empty() ) {
        return;
    }
    int len = musics.size();
    for ( int i = 0; i<len; i++ ) {
        QStringList tmp = musics[i].split("/");
        QStandardItem* item = new QStandardItem(QIcon(":/resource/music.ico"), tmp.back());
        this->m_musicListModel->appendRow(item);
    }
    this->_m_player->append(musics);
}

void MainWindow::About() {
    QMessageBox about(QMessageBox::Information, tr("About"),
                          tr("<h3>Music Player</h3>"
                             "<p>Author: Hejs</p>"
                             "<p>mail: 240197153@qq.com</p>"),
                      QMessageBox::Ok, this);
    about.exec();
}

void MainWindow::PlayMusic(const QModelIndex &index) {
    // 播放音乐事件
    this->_m_player->setPlayIndex(index.row());
    this->Play(true);
}

void MainWindow::Play( bool custom ) {
    bool play_flag = false;
    if ( custom ) {
        play_flag = true;
        this->_m_player->SetStatus(Player::PrePlay);
    }else {
        if ( custom == false && this->_m_player->GetStatus() == Player::Playing ) {
            this->_m_player->stop();
            this->m_rightWidget->ChangePlayBtnStatus(false);
        }else {
            play_flag = true;
        }
    }

    if ( play_flag ) {
        try {
            this->_m_player->play();
        }catch( char const* err ) {
            ShowToast(this, err);
            return;
        }
        this->m_rightWidget->ChangePlayBtnStatus(true);
    }
}

void MainWindow::Next() {
    this->_m_player->next();
    this->Play(true);
}

void MainWindow::Prev() {
    this->_m_player->prev();
    this->Play(true);
}

void MainWindow::positionChange(qint64 pos) {
    if (pos == 0 ) return;
    this->m_rightWidget->m_processBar->setValue(pos);
    int i, len;
    int sec = pos / 1000;
    int min = sec / 60;
    sec -= min * 60;
    QString show = QString("%1%2:%3%4").arg(min/10).arg(min%10).arg(sec/10).arg(sec%10);
    this->m_rightWidget->m_currentTime->setText(show);
    // 显示歌詞
    if ( this->_m_player->GetParserStatus() == LyricsParser::ParseEnd ) {
        len = this->_m_player->GetLyricsList()->size() - 1;
        for ( i = 0; i<len; ++i ) {
            if ( pos > this->_m_lyricsList->at(i).first && pos < this->_m_lyricsList->at(i+1).first ) break;
        }
        if ( i >= 1 ) {
            this->m_rightWidget->m_lyrics[1].setText(this->_m_lyricsList->at(i-1).second);
        }
        if ( i >= 2 ) {
            this->m_rightWidget->m_lyrics[0].setText(this->_m_lyricsList->at(i-2).second);
        }
        if ( i <= (len - 1))
            this->m_rightWidget->m_lyrics[3].setText(this->_m_lyricsList->at(i+1).second);
        else
            this->m_rightWidget->m_lyrics[3].setText("");

        if ( i <= (len - 2))
            this->m_rightWidget->m_lyrics[4].setText(this->_m_lyricsList->at(i+2).second);
        else
            this->m_rightWidget->m_lyrics[4].setText("");
        this->m_rightWidget->m_lyrics[2].setText(this->_m_lyricsList->at(i).second);
    }
}

void MainWindow::GetMetaInfo() {
    this->_m_player->SetLyricsStauts(LyricsParser::Parseing);

    qint64 total_ms = this->_m_player->GetPlayer()->duration();
    this->m_rightWidget->m_processBar->setRange(0, total_ms);
    this->_m_player->ResetPos();
    int sec = total_ms / 1000;
    int min = sec / 60;
    sec -= min * 60;
    QString show = QString("/ %1:%2").arg(min).arg(sec);
    this->m_rightWidget->m_totalTime->setText(show);

    for ( int i = 0; i<5; ++i ) {
        this->m_rightWidget->m_lyrics[i].setText(tr(""));
    }

    int index = this->_m_player->GetIndex();
    QModelIndex Index = this->m_musicListModel->index(index, 0);
    this->m_musicListWidget->setCurrentIndex(Index);
    this->_m_player->GetLrc();
}

void MainWindow::ChangePositionCustom() {
    int i = this->m_rightWidget->m_processBar->value();
    this->_m_player->SetPosition(i);
}

void MainWindow::timerEvent(QTimerEvent *event) {
    if ( this->m_toast != nullptr ) {
        delete this->m_toast;
        this->m_toast = nullptr;
    }
    this->removeItem(this->_m_player->GetIndex());
    event->accept();
}
