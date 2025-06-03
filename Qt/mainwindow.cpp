#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>
#include <QMessageBox>
#include <QDebug>
#include <QSerialPort>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c) {
            int idx = r*3 + c;
            cells[idx] = findChild<QPushButton*>(
                            QString("cell%1%2").arg(r).arg(c));
        }

    initPort();
    updateCursorLCD();
    redrawBoard();
    updateHighlight();
    resetGame();
}
MainWindow::~MainWindow() { delete ui; port->close(); }


void MainWindow::initPort() {
    this->port = new QSerialPort("COM2"); // COM2 should exist (create it with VSPE first)
    port->setBaudRate(QSerialPort::Baud9600);
    port->setDataBits(QSerialPort::Data8);
    port->setStopBits(QSerialPort::OneStop);
    port->open(QSerialPort::ReadWrite);
    if (port->isOpen()) {
        qDebug() << "Port is open!\n";
        connect(port, SIGNAL(readyRead()), this, SLOT(on_data_recieved()));
    } else {
        qDebug() << "Can't open port\n";
    }
}

void MainWindow::updateHighlight() {
    const QString colorX = "#ff8c00";   // оранжевый
    const QString colorO = "#0066ff";   // синий

    for (int i = 0; i < 9; ++i)
    {
        QString style;

        if      (board[i] == 1)  style += "color:" + colorX + ";";
        else if (board[i] == 2)  style += "color:" + colorO + ";";

        style += "font-size:28px;font-weight:bold;";

        if (i == cursor)
        {
            const QString borderColor = (player == 1) ? colorX : colorO;
            style += QString("border:3px solid %1;"
                             "background:#fffbe6;")
                             .arg(borderColor);
        }

        cells[i]->setStyleSheet(style);
    }
}

void MainWindow::on_data_recieved() {
    char buffer[16];
    if (port->bytesAvailable()) {
        int numRead = port->read(buffer, 10);
        qDebug() << "read " << numRead << " bytes from port\n";
        qDebug() << buffer << "\n";
    }
}


void MainWindow::on_btnInc_clicked() {
    cursor = (cursor + 1) % 9;
    updateCursorLCD();
}

void MainWindow::on_btnOk_clicked() {
    if (board[cursor] != 0)
        return;

    board[cursor] = player;
    redrawBoard();

    if (checkWin(player))
    {
        QMessageBox::information(this,
            "Победа", player == 1 ? "X выиграл!" : "O выиграл!");
        resetGame();
        return;
    }

    if (std::all_of(board.begin(), board.end(),
                    [](int v){ return v!=0; }))
    {
        QMessageBox::information(this,"Ничья","Победила дружба.");
        resetGame();
        return;
    }

    cursor = 0;
    switchPlayer();
    updateCursorLCD();
}

void MainWindow::on_cell_clicked() {
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    int idx = -1;
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            if (btn->objectName() == QString("cell%1%2").arg(r).arg(c))
                idx = r * 3 + c;

    if (idx == -1) return;
    cursor = idx;
    updateCursorLCD();
}

void MainWindow::updateCursorLCD() {
    ui->lcdCell->display(cursor);
    updateHighlight();
}

void MainWindow::redrawBoard() {
    for (int i = 0; i < 9; ++i)
        cells[i]->setText(board[i] == 1 ? "X" :
                          board[i] == 2 ? "O" : "");
}

void MainWindow::switchPlayer() {
    player = 3 - player;                // 1↔2
    ui->labelPlayer->setText(
        player == 1 ? "Ход: X" : "Ход: O");
}

bool MainWindow::checkWin(int p) const {
    static const int w[8][3] = { {0,1,2},{3,4,5},{6,7,8},
                                 {0,3,6},{1,4,7},{2,5,8},
                                 {0,4,8},{2,4,6} };
    for (auto &line : w)
        if (board[line[0]] == p &&
            board[line[1]] == p &&
            board[line[2]] == p) return true;
    return false;
}

void MainWindow::resetGame() {
    board.fill(0);
    cursor   = 0;
    player   = 1;
    updateCursorLCD();
    redrawBoard();
    ui->labelPlayer->setText("Ход: X");
}
