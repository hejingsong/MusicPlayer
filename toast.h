#ifndef TOAST_H
#define TOAST_H

#include <QLabel>
#include <QWidget>
#include <QHBoxLayout>

class Toast: public  QWidget
{
public:
    Toast( const QString &message, QWidget* parent = 0);
    ~Toast();
private:
    void    init(const QSize parentSize);

    QString m_message;
    QLabel* m_messageBox;
    QHBoxLayout* m_mainLayout;
};

#endif // TOAST_H
