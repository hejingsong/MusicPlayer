#include <QFile>
#include "config.h"

QString Lyrics::sm_getSongIdUrl = "http://musicmini.baidu.com/app/search/searchList.php?qword=%1&ie=utf-8&page=0";
QString Lyrics::sm_getLyricsUrl = "http://music.baidu.com/data/music/links?songIds=%1&ie=utf-8";
QString Lyrics::sm_lyricsDir = "./lyrics";
QString Lyrics::sm_startStr = "playSong(&#039;";
QString Lyrics::sm_endStr = "&#039;";

Lyrics::Lyrics(QObject *parent): QObject(parent) {
    this->m_manage = new QNetworkAccessManager;
    this->m_status = GetSongId;
    connect(this->m_manage, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    connect( this, SIGNAL(parseLyrics(QString)), parent, SLOT(parseLyrics( QString)) );
}

Lyrics::~Lyrics() {
    delete this->m_manage;
}

QJsonObject Lyrics::ChangeStringToJson( const QString str ) {
    QJsonDocument json = QJsonDocument::fromJson(str.toUtf8());
    if ( json.isNull() )
        throw "str error";
    return json.object();
}

void Lyrics::replyFinished(QNetworkReply* reply) {
    QByteArray readByte = reply->readAll();

    if ( this->m_status == GetSongId ) {
        int startLen = Lyrics::sm_startStr.length();
        int index = readByte.indexOf( Lyrics::sm_startStr );
        int index1 = readByte.indexOf( Lyrics::sm_endStr, index+startLen );
        if ( index < 0 || index1 < 0 ) {
            return;
        }
        QByteArray::const_iterator start = readByte.begin() + index + startLen;
        QByteArray::const_iterator end = readByte.begin() + index1;
        QString songId;
        for (; start!=end; ++start) {
            songId.push_back(*start);
        }
        QString url = Lyrics::sm_getLyricsUrl.arg(songId);
        this->m_manage->get( QNetworkRequest( QUrl::fromEncoded( url.toUtf8()) ) );
        this->m_status = GetLyricsUrl;
    }else if ( this->m_status == GetLyricsUrl ) {
        QJsonObject json;
        try {
            json = Lyrics::ChangeStringToJson( readByte );
        } catch ( char const* err ) {
#if Debug
            qDebug() << err;
#endif
            return;
        }
        if ( json["errorCode"].toInt() != 22000 ) {
#if Debug
            qDebug() << "获取失败";
#endif
            return;
        }
        QString lyricsUrl = json["data"].toObject()["songList"].toArray()[0].toObject()["lrcLink"].toString();
        this->m_manage->get(  QNetworkRequest( QUrl::fromEncoded( lyricsUrl.toUtf8()) ) );
        this->m_status = GetLyrics;
    }else {
        QString fileName = Lyrics::sm_lyricsDir + "/" + this->m_query + ".lrc";
        QFile file(fileName);
        file.open(QFile::WriteOnly);
        file.write(readByte);
        file.close();
        this->m_status = GetSongId;
        emit parseLyrics(fileName);
    }
}

void Lyrics::GetLrc(const QString query) {
    QString lyricsFile = QString("%1/%2.lrc").arg(Lyrics::sm_lyricsDir).arg(query);
    if ( Player::isFile(lyricsFile) ) {
        emit parseLyrics(lyricsFile);
        return;
    }
    this->m_query = query;
    QString url = Lyrics::sm_getSongIdUrl.arg(this->m_query);
    this->m_manage->get(QNetworkRequest( QUrl::fromEncoded(url.toUtf8()) ));
}

LyricsParser::LyricsParser() {
    this->m_status = Parseing;
}

LyricsParser::~LyricsParser() {

}

void LyricsParser::parse( const QString lyricsFile ) {
    QFile file( lyricsFile );
    QByteArray readBuf;

    this->m_status = Parseing;
    this->m_lyricsList.clear();

    if ( !file.exists() ) return;

    file.open(QFile::ReadOnly);
    while ( !file.atEnd() ) {
        readBuf = file.readLine(1024);
        if ( readBuf[0] == '\n' ) continue;
        std::pair<int, QString> tmp = this->parseLine( readBuf );
        if ( tmp.first == -1 ) continue;
        this->m_lyricsList.push_back(tmp);
    }
    this->m_status = ParseEnd;
}

std::pair<int, QString> LyricsParser::parseLine( QString line ) {
    QString tmp;
    int min, sec, msec, mtime;
    QString::iterator start = line.begin() + 1;
    QString::iterator end = start + 2;
    int index = line.indexOf('[');
    int index1 = line.indexOf(']');
    QString::iterator it = line.begin() + index1 + 1;
    // 不是[xx:xx:xx]格式
    if ( (index1 - index) < 8 ) {
        return std::pair<int, QString>(-1, "");
    }
    // 是[xx:xx:xx]\n格式
    for (; it!=line.end(); ++it) {
        tmp.push_back(*it);
    }
    if ( tmp.compare("\n") == 0 ) {
        return std::pair<int, QString>(-1, "");
    }
    tmp.clear();
    // 正常格式
    for ( ; start!=end; ++start ) {
        tmp.push_back(*start);
    }
    min = tmp.toInt();
    tmp.clear();

    start += 1;
    end = start + 2;
    for ( ; start!=end; ++start ) {
        tmp.push_back(*start);
    }
    sec = tmp.toInt();
    tmp.clear();

    start += 1;
    end = start + 2;
    for ( ; start!=end; ++start ) {
        tmp.push_back(*start);
    }
    msec = tmp.toInt();
    tmp.clear();

    mtime = min * 60000 + sec * 1000 + msec;

    start += 1;
    end = line.end();
    for ( ; start!=end; ++start ) {
        tmp.push_back(*start);
    }
#if Debug
    qDebug() << mtime << " " << tmp;
#endif
    return std::pair<int, QString>(mtime, tmp);
}

Player::Player(QObject* parent): QObject(parent) {
    this->m_playlist = new QMediaPlaylist;
    this->m_player = new QMediaPlayer;
    this->m_lyrics = new Lyrics(this);
    this->m_size = 0;
    this->m_index = 0;
    this->m_status = PrePlay;
    this->m_pos = 0;
    this->m_parse = new LyricsParser();

    this->m_playlist->setPlaybackMode(QMediaPlaylist::Loop);
    this->m_player->setPlaylist(this->m_playlist);
}

Player::Player(const QStringList &obj, QObject *parent): QObject(parent)
{
    this->m_playlist = new QMediaPlaylist;
    this->m_player = new QMediaPlayer;
    foreach( QString m, obj ) {
        this->m_playlist->addMedia(QUrl(m));
    }
    this->m_size = obj.size();
    this->m_index = 0;
    this->m_status = PrePlay;
    this->m_pos = 0;
    this->m_parse = new LyricsParser();

    this->m_playlist->setPlaybackMode(QMediaPlaylist::Loop);
    this->m_player->setPlaylist(this->m_playlist);
}

Player::~Player() {
    delete this->m_playlist;
    delete this->m_player;
    delete this->m_lyrics;
}

void Player::setPlayIndex(const int index) {
    this->m_index = index;
}

QString Player::currentMediaName( const int index ) {
    QString musicUrl = this->m_playlist->media(index).canonicalUrl().toString();
    if ( !this->isFile(musicUrl) ) {
        this->m_playlist->removeMedia(index);
        this->m_status = CurrentError;
        throw "文件不存在";
    }

    return musicUrl.split("/").back().split(".").front();
}

bool Player::append(const QStringList& musics) {
    foreach(QString m, musics) {
        this->m_playlist->addMedia(QUrl(m));
    }
    this->m_size += musics.size();
    return true;
}

bool Player::remove(int index) {
    this->m_playlist->removeMedia(index);
    this->m_size--;
    return true;
}

bool Player::isFile(const QString file) {
    QFile f(file);
    return f.exists();
}

void Player::GetLrc() {
    QString musicName = this->currentMediaName(this->m_index);
    this->m_lyrics->GetLrc(musicName);
}

bool Player::play() {
    if ( !this->m_playlist->isEmpty() ) {
        if ( this->m_status == PrePlay )
            this->m_playlist->setCurrentIndex(this->m_index);
        this->m_player->setPosition(this->m_pos);
        this->m_player->play();
        this->m_status = Playing;
        return true;
    }else {
        return false;
    }
}

bool Player::stop() {
    this->m_pos = this->m_player->position();
    this->m_player->pause();
    this->m_status = Stop;
    return true;
}

bool Player::prev() {
    if ( --this->m_index < 0 )
        this->m_index = this->m_size - 1;
    return true;
}

bool Player::next() {
    if ( this->m_status == CurrentError )
        return true;

    if ( ++this->m_index == this->m_size )
        this->m_index = 0;
    return true;
}

void Player::parseLyrics( const QString lyricsFile ) {
    this->m_parse->parse(lyricsFile);
}
