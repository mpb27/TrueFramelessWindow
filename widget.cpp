#include "widget.h"

#include <QApplication>
#include <QLabel>
#include <QLayout>
#include <QStyle>
#include <QtWidgets>

#include <fmt/format.h>

Widget::Widget(QWidget *parent)
	: QMainWindow(parent)
{

	//Set a black background for funsies
	QPalette Pal(palette());
	//Pal.setColor(QPalette::Background, Qt::blue);
	setAutoFillBackground(true);
	setPalette(Pal);

	//Windows example of adding a toolbar + min/max/close buttons
#ifdef _WIN32

	//Add the toolbar
	toolBar = new QToolBar(this);
    //toolBar->setStyleSheet("QToolBar{background-color: lightGray; border: none;}");
    toolBar->setStyleSheet("QToolBar{background-color: none; border: none;}");
	toolBar->setMovable(false);
	toolBar->setFloatable(false);
	addToolBar(toolBar);

	//Create a transparent-to-mouse-events widget that pads right for a fixed width equivalent to min/max/close buttons
	/*
	QWidget* btnSpacer = new QWidget(toolBar);
	btnSpacer->setAttribute(Qt::WA_TransparentForMouseEvents);
	btnSpacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	btnSpacer->setStyleSheet("background-color: none; border: none;");
	btnSpacer->setFixedWidth(135);  // rough width of close/min/max buttons
	toolBar->addWidget(btnSpacer);
    */
	QLabel* windowIcon = new QLabel();
	windowIcon->setFixedSize(24, 24);
	windowIcon->setStyleSheet("background: red;");
	windowIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
	windowIcon->setAlignment(Qt::AlignCenter);
	//windowIcon->setPixmap(QApplication::style()->standardIcon((QStyle::StandardPixmap)0).pixmap(16, 16));
	windowIcon->setPixmap(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_TitleBarMenuButton).pixmap(16, 16));
    toolBar->addWidget(windowIcon);

    //toolBar->addWidget();

    // Add Menu Bar.
    {
        QMenuBar* menuBar = new QMenuBar(toolBar);
        menuBar->setMinimumWidth(10);
        menuBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        //menuBar->setStyleSheet(QString("color:%1;").arg(theme["text"].name()));
        menuBar->setStyleSheet("QMenuBar { min-height: 24px; }");
        QMenu* fileMenu = new QMenu("File", this);
        //fileMenu->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        menuBar->addMenu(fileMenu);
        QMenu* cpuMenu = new QMenu("CPU", this);
        menuBar->addMenu(cpuMenu);
        QMenu* gpuMenu = new QMenu("GPU", toolBar);
        menuBar->addMenu(gpuMenu);
        QMenu* windowMenu = new QMenu("Window");
        menuBar->addMenu(windowMenu);
        QMenu* helpMenu = new QMenu("Help");
        menuBar->addMenu(helpMenu);
        menuBar->addAction("Hello");
        fileMenu->addAction("Save");
        fileMenu->addAction("Exit");

        toolBar->addWidget(menuBar);

    }



	//Create a title label just because
	QLabel* titleLabel = new QLabel("TrueFramelessWindow");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	//titleLabel->setFixedWidth(160);
	titleLabel->setStyleSheet("color:ffffff;");

	//Set it transparent to mouse events such that you can click and drag when moused over the label
	titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
	toolBar->addWidget(titleLabel);

	//Create spacer widgets to keep the title centered
	/*
	QWidget* leftSpacer = new QWidget(toolBar);
    leftSpacer->setAttribute(Qt::WA_TransparentForMouseEvents);
    leftSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    leftSpacer->setStyleSheet("background-color: none; border: none;");
	*/

	QWidget* rightSpacer = new QWidget(toolBar);

	//Set them transparent to mouse events + auto-expanding in size
	rightSpacer->setAttribute(Qt::WA_TransparentForMouseEvents);
	rightSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	rightSpacer->setStyleSheet("background-color: none; border: none;");

	//Add spacers & title label
	//toolBar->addWidget(leftSpacer);
	//toolBar->addWidget(titleLabel);
	//toolBar->addWidget(rightSpacer);


	//Create the min/max/close buttons
	minimizeButton = new QToolButton();  minimizeButton->setText("-");
	maximizeButton = new QToolButton();  maximizeButton->setText("O");
	closeButton    = new QToolButton(); // closeButton->setText("X");
    closeButton->setStyleSheet("QToolButton:hover { background-color: red; border: 0px solid lightgray; }");

	/*
	auto svg2Pixmap = [](const QByteArray& svgContent,
                         const QSize& size,
                         QPainter::CompositionMode mode)
    {
        QSvgRenderer rr(svgContent);
        QImage image(size.width(), size.height(), QImage::Format_ARGB32);
        QPainter painter(&image);
        painter.setCompositionMode(mode);
        image.fill(Qt::transparent);
        rr.render(&painter);
        return QPixmap::fromImage(image);
    };
    */


	auto closeIcon = QIcon(
	QPixmap::fromImage(
	QImage::fromData("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"12\" height=\"12\" viewBox=\"0 0 12 12\"><polygon fill=\"#ff00ff\" fill-rule=\"evenodd\" points=\"11 1.576 6.583 6 11 10.424 10.424 11 6 6.583 1.576 11 1 10.424 5.417 6 1 1.576 1.576 1 6 5.417 10.424 1\"></polygon></svg>")
    )
    );

    auto closeIcon2 = QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_TitleBarCloseButton);

    closeButton->setIcon(closeIcon2);
    //closeButton->setIconSize(QSize(12, 12));


	minimizeButton->setAutoRaise(true);
	maximizeButton->setAutoRaise(true);
	closeButton->setAutoRaise(true);

	maximizeButton->setCheckable(true);  // To allow for two draw states, maximize and restore.

	minimizeButton->setFixedSize(45, 28);
	maximizeButton->setFixedSize(45, 28);
	closeButton->setFixedSize(45, 28);

	toolBar->addWidget(minimizeButton);
	toolBar->addWidget(maximizeButton);
	toolBar->addWidget(closeButton);

	/*
	toolBar->layout()->setAlignment(minimizeButton, Qt::AlignTop);
	toolBar->layout()->setAlignment(maximizeButton, Qt::AlignTop);
	toolBar->layout()->setAlignment(closeButton, Qt::AlignTop);
	*/

	//An actual app should use icons for the buttons instead of text
	//and style the different button states / widget margins in css




#endif

}
