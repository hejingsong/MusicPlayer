#include "toast.h"

Toast::Toast(const QString& message, QWidget *parent): QWidget(parent)
{
    this->m_message = message;
    this->init(parent->size());
}

Toast::~Toast() {

}

void Toast::init(const QSize parentSize) {
    this->m_messageBox = new QLabel(this->m_message, this);
    this->m_mainLayout = new QHBoxLayout;

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setStyleSheet("background: rgba(30, 30, 30, 0.3); border-radius: 5px; color: #ccc; font-family: Consolas;");
    this->m_mainLayout->setContentsMargins(0, 0, 0, 0);

    this->m_messageBox->setAlignment(Qt::AlignCenter);
    this->m_mainLayout->addWidget(this->m_messageBox);

    this->setLayout(this->m_mainLayout);


    this->move(parentSize.width()/2 + 50, parentSize.height() - 50);
}
