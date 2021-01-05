## Qt之自定义界面

### 1. 实现无边框加可移动界面

设置窗口标志为无边框的语句：

```C++
setWindowFlags(Qt::FramelessWindowHint | windowFlags());
```

关于WindowFlags枚举值很多，其中有一个`WindowStaysOnTopHint`，可以让窗体位于所有界面之上，没有标题栏和状态栏。设置无边框之后，窗体是无法移动的，我们可以通过**重载鼠标事件处理函数**来实现，或者使用Windows系统消息实现。

```C++
// 鼠标相对于窗体的位置 event->globalPos() - this->pos()
void Widget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_bPressed = true;
        m_point = event->pos();  // QPoint类型
    }
}
// 若鼠标左键被按下，移动窗体位置
void Widget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_bPressed)
        move(event->pos() - m_point + pos());
}
// 设置鼠标未被按下
void Widget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    m_bPressed = false;
}
```

Win API实现方式：

```C++
// 包含头文件与需要用到的库
#ifdef Q_OS_WIN
#pragma comment(lib, "user32.lib")
#include <qt_windows.h>
#endif

// 发送系统消息，进行移动
void Widget::mousePressEvent(QMouseEvent *event)
{
#ifdef Q_OS_WIN
    if (ReleaseCapture())
           SendMessage(HWND(winId()), WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);
       event->ignore();
#else
#endif
}
```

因为没有了状态栏，窗口自带的最小化和关闭按键也没有了，自己实现即可。

### 2. 添加自定义标题栏

我们实现了自定义窗体的移动，但是我们缺少一个标题栏来显示窗体的图标、标题，以及控制窗体最小化、最大化、关闭的按钮。自定义标题栏后，所有的控件我们都可以定制，比如：在标题栏中添加换肤、设置按钮以及其他控件。

自定义标题栏头文件：

```C++
#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>

class QLabel;
class QPushButton;

class TitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent = nullptr);

protected:
    // 双击标题栏进行界面的最大化/还原
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    // 进行鼠界面的拖动
    virtual void mousePressEvent(QMouseEvent *event);
    // 设置界面标题与图标
    virtual bool eventFilter(QObject *obj, QEvent *event);

private:
    // 最大化/还原
    void updateMaximize();

private slots:

    // 进行最小化、最大化/还原、关闭操作
    void onClicked();

private:
    QLabel *m_pIconLabel;
    QLabel *m_pTitleLabel;
    QPushButton *m_pMinimizeButton;
    QPushButton *m_pMaximizeButton;
    QPushButton *m_pCloseButton;
};

#endif // TITLEBAR_H

```

自定义标题栏源文件：

```C++

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include "TitleBar.h"
#include <QIcon>

#ifdef Q_OS_WIN
#pragma comment(lib, "user32.lib")
#include <qt_windows.h>
#endif

TitleBar::TitleBar(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(30);

    m_pIconLabel = new QLabel(this);
    m_pTitleLabel = new QLabel(this);
    m_pMinimizeButton = new QPushButton(this);
    m_pMaximizeButton = new QPushButton(this);
    m_pCloseButton = new QPushButton(this);

    m_pIconLabel->setFixedSize(20, 20);
    m_pIconLabel->setScaledContents(true);

    m_pTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_pMinimizeButton->setFixedSize(27, 22);
    m_pMaximizeButton->setFixedSize(27, 22);
    m_pCloseButton->setFixedSize(27, 22);
    m_pMinimizeButton->setIcon(QIcon(":/title/picture/minimize.png"));
    m_pMaximizeButton->setIcon(QIcon(":/title/picture/maximize.png"));
    m_pCloseButton->setIcon(QIcon(":/title/picture/close.png"));

    m_pTitleLabel->setObjectName("whiteLabel");
    m_pMinimizeButton->setObjectName("minimizeButton");
    m_pMaximizeButton->setObjectName("maximizeButton");
    m_pCloseButton->setObjectName("closeButton");

    m_pMinimizeButton->setToolTip("Minimize");
    m_pMaximizeButton->setToolTip("Maximize");
    m_pCloseButton->setToolTip("Close");

    QHBoxLayout *pLayout = new QHBoxLayout(this);
    pLayout->addWidget(m_pIconLabel);
    pLayout->addSpacing(5);
    pLayout->addWidget(m_pTitleLabel);
    pLayout->addWidget(m_pMinimizeButton);
    pLayout->addWidget(m_pMaximizeButton);
    pLayout->addWidget(m_pCloseButton);
    pLayout->setSpacing(0);
    pLayout->setContentsMargins(5, 0, 5, 0);

    setLayout(pLayout);

    connect(m_pMinimizeButton, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
    connect(m_pMaximizeButton, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
    connect(m_pCloseButton, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    emit m_pMaximizeButton->clicked();
}

void TitleBar::mousePressEvent(QMouseEvent *event)
{
#ifdef Q_OS_WIN
    if (ReleaseCapture())
    {
        QWidget *pWindow = this->window();
        if (pWindow->isTopLevel())
        {
           SendMessage(HWND(pWindow->winId()), WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);
        }
    }
       event->ignore();
#else
#endif
}

bool TitleBar::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type())
    {
    case QEvent::WindowTitleChange:
    {
        QWidget *pWidget = qobject_cast<QWidget *>(obj);
        if (pWidget)
        {
            m_pTitleLabel->setText(pWidget->windowTitle());
            return true;
        }
    }
    case QEvent::WindowIconChange:
    {
        QWidget *pWidget = qobject_cast<QWidget *>(obj);
        if (pWidget)
        {
            QIcon icon = pWidget->windowIcon();
            m_pIconLabel->setPixmap(icon.pixmap(m_pIconLabel->size()));
            return true;
        }
    }
    case QEvent::WindowStateChange:
    case QEvent::Resize:
        updateMaximize();
        return true;

    default:
        break;
    }
    return QWidget::eventFilter(obj, event);
}

void TitleBar::onClicked()
{
    QPushButton *pButton = qobject_cast<QPushButton *>(sender());
    QWidget *pWindow = this->window();
    if (pWindow->isTopLevel())
    {
        if (pButton == m_pMinimizeButton)
        {
            pWindow->showMinimized();
        }
        else if (pButton == m_pMaximizeButton)
        {
            pWindow->isMaximized() ? pWindow->showNormal() : pWindow->showMaximized();
        }
        else if (pButton == m_pCloseButton)
        {
            pWindow->close();
        }
    }
}

// 因为窗体大小发生变化的时候，最大化的图标、提示应该对应的发生变化
void TitleBar::updateMaximize()
{
    QWidget *pWindow = this->window();
    if (pWindow->isTopLevel())
    {
        bool bMaximize = pWindow->isMaximized();
        if (bMaximize)
        {
            m_pMaximizeButton->setToolTip(tr("Restore"));
            m_pMaximizeButton->setProperty("maximizeProperty", "restore");
        }
        else
        {
            m_pMaximizeButton->setProperty("maximizeProperty", "maximize");
            m_pMaximizeButton->setToolTip(tr("Maximize"));
        }

        m_pMaximizeButton->setStyle(QApplication::style());
    }
}

```

相关说明：

- mousePressEvent：之前，我们将界面移动的事件写在主界面里面，这会有一个问题，一般情况下，是界面随着标题栏的移动而移动，而并非界面中的所有位置都可以进行拖动，所以我们将事件写在标题栏中比较合理。
- mouseDoubleClickEvent：双击标题栏会进行窗体的最大化/还原，所以我们需要重写此事件进行控制。
- eventFilter：事件过滤器，这里被监听的窗体为标题栏所在的窗体，所以当窗体标题、图标等信息发生改变时，标题栏也应该随之改变。最好不要通过直接调用接口的形式来操作对应的行为，比如：TitleBar中定义一个public函数来专门修改标题与图标，这样会造成不必要的麻烦，因为Qt本身就是基于事件的，所以此处采用过滤器的方式。
- updateMaximize：因为窗体大小发生变化的时候，最大化的图标、提示应该对应的发生变化，所以在eventFilter中事件触发时调用。

使用方式举例：

```C++
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());

    TitleBar *pTitleBar = new TitleBar(this);
    installEventFilter(pTitleBar);

    setWindowTitle("Custom Window");
    setWindowIcon(QIcon(":/Images/logo"));

    QVBoxLayout *pLayout = new QVBoxLayout();
    pLayout->addWidget(pTitleBar);
    pLayout->addStretch();
    pLayout->setSpacing(0);
    pLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(pLayout);
}
```

### 3. 