#ifndef PLAYER_H
#define PLAYER_H

#include <vector>
#include <QObject>
#include <QString>
#include <QtNetwork>
#include <QJsonObject>
#include <QStringList>
#include <QMediaPlayer>
#include <QJsonDocument>
#include <QMediaPlaylist>

/**
 * @brief The Lyrics class
 * 用于获取歌词并且保存为文件
 */
class Lyrics: public QObject {
    Q_OBJECT
public:
    enum LyricsStatus {
        GetSongId = 0,
        GetLyricsUrl,
        GetLyrics
    };
public:
    Lyrics(QObject *parent = 0);
    ~Lyrics();
    static QJsonObject ChangeStringToJson( const QString str );
    void GetLrc(const QString query);

    static QString sm_lyricsDir;
private:
    static QString sm_getSongIdUrl;
    static QString sm_getLyricsUrl;
    static QString sm_startStr;
    static QString sm_endStr;
    QString m_query;
    LyricsStatus m_status;
    QNetworkAccessManager* m_manage;
private slots:
    void replyFinished(QNetworkReply* );
signals:
    void parseLyrics( const QString );
};

/**
 * @brief The LyricsParser class
 * 歌词解析器
 */
class LyricsParser {
public:
    enum ParseStatus {
        Parseing = 0,
        ParseEnd
    };
public:
    explicit LyricsParser();
    ~LyricsParser();
    void parse(const QString lyricsFile);
    inline void SetStatus( ParseStatus status ) {
        this->m_status = status;
    }
    inline ParseStatus GetStatus() {
        return this->m_status;
    }
    inline const std::vector<std::pair<int, QString> >* GetLyricsList() {
        return &(this->m_lyricsList);
    }
private:
    std::vector<std::pair<int, QString>> m_lyricsList;
    ParseStatus m_status;

    std::pair<int, QString> parseLine( QString line );
};

/**
 * @brief The Player class
 * 用于播放音乐 对播放器的控制
 */
class Player: public QObject
{
    Q_OBJECT
public:
    enum PlayStatus{
        Playing = 0,
        Stop,
        PrePlay,
        CurrentError
    };
public:
    Player(QObject* parent = 0);
    explicit Player(const QStringList& obj, QObject* parent = 0);
    ~Player();
    void setPlayIndex(const int index);
    bool append(const QStringList &musics);
    bool remove(int index);
    bool play();
    bool stop();
    bool prev();
    bool next();
    void GetLrc();
    QString currentMediaName(const int index);

    static bool isFile(const QString file);

    inline void ResetPos() {
        this->m_pos = 0;
    }
    inline PlayStatus GetStatus() {
        return this->m_status;
    }
    inline void SetStatus( PlayStatus status ) {
        this->m_status = status;
    }
    inline QMediaPlayer* GetPlayer() {
        return this->m_player;
    }
    inline void SetPosition(const qint64 pos) {
        this->m_player->setPosition(pos);
    }
    inline int GetIndex() {
        if (this->m_player->state() == QMediaPlayer::PlayingState)
            this->m_index = this->m_playlist->currentIndex();
        return this->m_index;
    }
    inline void SetLyricsStauts( LyricsParser::ParseStatus status ) {
        this->m_parse->SetStatus(status);
    }
    inline LyricsParser::ParseStatus GetParserStatus() {
        return this->m_parse->GetStatus();
    }
    inline const std::vector<std::pair<int, QString> >* GetLyricsList() {
        return this->m_parse->GetLyricsList();
    }

private:
    QMediaPlaylist* m_playlist;
    QMediaPlayer* m_player;
    PlayStatus m_status;
    int m_index;    // 播放的节点
    int m_size;
    qint64 m_pos;   // 播放位置
    Lyrics* m_lyrics;
    LyricsParser* m_parse;

private slots:
    void parseLyrics( const QString lyricsFile );
};

#endif // PLAYER_H
