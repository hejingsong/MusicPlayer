#include <QResizeEvent>
#include "rightwidget.h"

RightWidget::RightWidget(QWidget* parent):
    QWidget(parent)
{
    this->init_widget();
}

RightWidget::~RightWidget() {
    delete [] this->m_lyrics;
}

void RightWidget::init_widget() {
    this->m_mainLayout = new QVBoxLayout;
    this->m_mainLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(this->m_mainLayout);

    this->init_topWidget();
    this->init_bottomWidget();

    this->setStyleSheet("background-color: #333333; color: white; font-size: 18px; font-family: Consolas;");
}

void RightWidget::init_topWidget() {
    this->m_topLayout = new QVBoxLayout;
    this->m_lyrics = new QLabel[5];

    this->m_topLayout->setContentsMargins(0, 0, 0, 0);

    for (int i=0; i<5; i++) {
        this->m_lyrics[i].setStyleSheet("QLabel{ color: #cccccc; }");
        this->m_lyrics[i].setMinimumHeight(100);

        this->m_lyrics[i].setAlignment(Qt::AlignCenter);
        this->m_lyrics[i].setParent(this);
        this->m_lyrics[i].setText("");
        this->m_topLayout->addWidget(&(this->m_lyrics[i]));
    }
    this->m_lyrics[2].setStyleSheet("QLabel{ font-size: 20px; }");

    this->m_mainLayout->addLayout(this->m_topLayout);
    this->m_mainLayout->addStretch();
}

void RightWidget::init_bottomWidget() {
    this->m_bottomLayout = new QHBoxLayout;
    this->m_prevBtn = new QPushButton("", this);
    this->m_nextBtn = new QPushButton("", this);
    this->m_stopBtn = new QPushButton("", this);
    this->m_processBar = new QSlider(Qt::Horizontal, this);
    this->m_totalTime = new QLabel("/ --:--", this);
    this->m_currentTime = new QLabel("00:00", this);

    this->m_prevBtn->setFixedSize(32, 32);
    this->m_stopBtn->setFixedSize(32, 32);
    this->m_nextBtn->setFixedSize(32, 32);

    this->m_prevBtn->setStyleSheet("QPushButton{ border: none; background: url(:/resource/prev_noactive.png);}"
                                   "QPushButton:hover{ background: url(:/resource/prev_active.png); }");
    this->m_stopBtn->setStyleSheet("QPushButton{ border: none; background: url(:/resource/play_noactive.png);}"
                                   "QPushButton:hover{ background: url(:/resource/play_active.png); }");
    this->m_nextBtn->setStyleSheet("QPushButton{ border: none; background: url(:/resource/next_noactive.png);}"
                                   "QPushButton:hover{ background: url(:/resource/next_active.png); }");

    this->m_processBar->setStyleSheet("QSlider::groove:horizontal{ border: 1px solid #999999; height: 1px; margin: 0px 0; left: 5px; right: 5px; }"
                                      "QSlider::handle:horizontal{ border: 0px; border-image: url(:/resource/circle.png); width: 15px; margin: -7px; }"
                                      "QSlider::sub-page:horizontal{ background: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0, stop:0 #439cf3, stop:0.25 #439cf3, stop:0.5 #439cf3, stop:1 #439cf3);}");

    this->m_bottomLayout->addWidget(this->m_prevBtn);
    this->m_bottomLayout->addWidget(this->m_stopBtn);
    this->m_bottomLayout->addWidget(this->m_nextBtn);
    this->m_bottomLayout->addWidget(this->m_processBar);
    this->m_bottomLayout->addWidget(this->m_currentTime);
    this->m_bottomLayout->addWidget(this->m_totalTime);

    this->m_mainLayout->addLayout(this->m_bottomLayout);
}

void RightWidget::ChangePlayBtnStatus(bool status) {
    if ( status ){
        this->m_stopBtn->setStyleSheet("QPushButton{ border: none; background: url(:/resource/stop_noactive.png);}"
                                   "QPushButton:hover{ background: url(:/resource/stop_active.png); }");
    }else {
        this->m_stopBtn->setStyleSheet("QPushButton{ border: none; background: url(:/resource/play_noactive.png);}"
                                   "QPushButton:hover{ background: url(:/resource/play_active.png); }");
    }
}

void RightWidget::resizeEvent(QResizeEvent *event) {
    QSize size = event->size();
    int height = ( size.height() - 35 ) / 5;
    for ( int i = 0; i<5; i++ ) {
        this->m_lyrics[i].setMinimumHeight(height);
    }
}
