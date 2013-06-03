#ifndef LABEL_HPP
#define LABEL_HPP

#include <QLabel>
#include <QMouseEvent>

// http://qt-project.org/wiki/Make-a-QLabel-Clickable
class ClickableLabel : public QLabel {
    Q_OBJECT
public:
    ClickableLabel(const QString& text, QWidget* parent=nullptr)
        : QLabel(text, parent)
    {
    }

signals:
    void clicked();
    void rightClicked();

protected:
    void mousePressEvent(QMouseEvent* event) {
        if (event->button() == Qt::MouseButton::RightButton)
            emit rightClicked();
        else if (event->button() == Qt::MouseButton::LeftButton)
            emit clicked();
    }
};

#endif // LABEL_HPP
