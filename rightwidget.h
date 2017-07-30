#ifndef RIGHTWIDGET_H
#define RIGHTWIDGET_H

#include <QLabel>
#include <QWidget>
#include <QLayout>
#include <QSlider>
#include <QTextEdit>
#include <QPushButton>

class RightWidget: public QWidget
{
public:
    RightWidget(QWidget* parent = nullptr);
    ~RightWidget();
public:
    QPushButton *m_prevBtn;
    QPushButton *m_nextBtn;
    QPushButton *m_stopBtn;
    QSlider     *m_processBar;
    QLabel      *m_currentTime;
    QLabel      *m_totalTime;
    QLabel      *m_lyrics;

    void        ChangePlayBtnStatus(bool status);
private:
    void    init_widget();
    void    init_topWidget();
    void    init_bottomWidget();
    void    resizeEvent(QResizeEvent *event);
private:
    QVBoxLayout *m_mainLayout;
    QVBoxLayout *m_topLayout;
    QHBoxLayout *m_bottomLayout;
};

#endif // RIGHTWIDGET_H
