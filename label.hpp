#ifndef LABEL_HPP
#define LABEL_HPP

#include <QLabel>
#include <QMouseEvent>

class ClickableLabel : public QLabel {
    Q_OBJECT
public:
    ClickableLabel(const QString& text, QWidget* parent=nullptr)
        : QLabel(text, parent)
    {
    }

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event) {
        if (event->button() == Qt::MouseButton::RightButton)
            emit clicked();
    }
};

#endif // LABEL_HPP
