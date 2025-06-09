#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdint.h>

#include <QPushButton>
#include <QMessageBox>
#include <QDebug>
#include <QSerialPort>
#include <QByteArray>
#include <QFile>
#include <QXmlStreamWriter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    for (int h = 0; h < 3; ++h)
    {
        QGridLayout *g = findChild<QGridLayout*>(QString("gridHist%1").arg(h));
        for (int i = 0; i < 9; ++i)
        {
            int r = i / 3;           // 0‥2
            int c = i % 3;           // 0‥2
            histCells[h][i] = qobject_cast<QLabel*>(
                                 g->itemAtPosition(r, c)->widget());
        }
    }

    loadHistory();
    refreshHistoryBoards();

    currentGame.clear();

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
    QByteArray bytes = port->readAll();
    uint8_t data = bytes[0];
    uint8_t cursor_shift = data & (1 << 7);
    uint8_t select = data & (1 << 6);
    if (cursor_shift) {
        on_btnInc_clicked();
    } else if (select) {
        on_btnOk_clicked();
    }
}

void MainWindow::on_btnInc_clicked() {
    cursor = (cursor + 1) % 9;
    updateCursorLCD();
}

void MainWindow::on_btnOk_clicked() {
    if (board[cursor] != 0)
        return;

    currentGame << Move{player, cursor};

    board[cursor] = player;
    redrawBoard();

    auto finishGame = [this]{
            history.prepend(currentGame);        // кладём вперёд
            while (history.size() > 3)           // максимум три игры
                history.removeLast();
            saveHistory();                       // history.xml
            currentGame.clear();                 // готовимся к новой партии
            refreshHistoryBoards();
        };

    if (checkWin(player))
    {
        QMessageBox::information(this,
            "Победа", player == 1 ? "X выиграл!" : "O выиграл!");
        finishGame();
        resetGame();
        return;
    }

    if (std::all_of(board.begin(), board.end(),
                    [](int v){ return v!=0; }))
    {
        QMessageBox::information(this,"Ничья","Победила дружба.");
        finishGame();
        resetGame();
        return;
    }

    sendMoveData(player, cursor);
    cursor = 0;
    switchPlayer();
    updateCursorLCD();
}

void MainWindow::saveHistory() const
{
    QFile f("history.xml");
    if (!f.open(QIODevice::WriteOnly|QIODevice::Truncate)) return;

    QXmlStreamWriter w(&f);
    w.setAutoFormatting(true);
    w.writeStartDocument();
    w.writeStartElement("history");

    for (const Game& g : history) {
        w.writeStartElement("game");
        for (const Move& m : g) {
            w.writeStartElement("move");
            w.writeAttribute("player", QString::number(m.player));
            w.writeAttribute("cell",   QString::number(m.cell));
            w.writeEndElement();                // </move>
        }
        w.writeEndElement();                    // </game>
    }
    w.writeEndElement();                        // </history>
}

void MainWindow::loadHistory()
{
    QFile f("history.xml");
    if (!f.open(QIODevice::ReadOnly)) return;

    QXmlStreamReader r(&f);
    history.clear();  Game g;

    while (!r.atEnd()) {
        r.readNext();
        if (r.isStartElement()) {
            if (r.name()=="game") g.clear();
            else if (r.name()=="move") {
                int pl  = r.attributes().value("player").toInt();
                int cel = r.attributes().value("cell").toInt();
                g << Move{pl, cel};
            }
        } else if (r.isEndElement() && r.name()=="game") {
            history << g;
        }
    }
}

void MainWindow::on_cell_clicked()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    int idx = std::find(cells.begin(), cells.end(), btn) - cells.begin();
    if (idx < 0 || idx >= 9) return;

    cursor = idx;
    updateCursorLCD();

    char data = static_cast<char>((1 << 6) | cursor);
    port->write(QByteArray(1, data));
}

void MainWindow::sendMoveData(int shape, int cell) {
    uint8_t data = 0;
    if (shape == 2) {
        data |= (1 << 4);
    }
    data |= cell;
    QByteArray ba(1, static_cast<char>(data));
    port->write(ba);
}

void MainWindow::updateCursorLCD() {
    ui->lcdCell->display(cursor + 1);
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

    char reset = 0 | (1 << 7);
    port->write(QByteArray(1, reset));
}

void MainWindow::refreshHistoryBoards()
{
    const QString colX = "#ff8c00", colO = "#0066ff";

    for (int h = 0; h < 3; ++h)
        for (int i = 0; i < 9; ++i)
        {
            QLabel *lab = histCells[h][i];
            if (h >= history.size()) { lab->clear(); continue; }

            const Game &g = history[h];              // 0-я – самая свежая
            int mark = 0;
            for (const Move &m : g)
                if (m.cell == i) mark = m.player;    // последний ход определяет

            if (mark == 1) {
                lab->setText("X");
                lab->setStyleSheet("color:"+colX);
            } else if (mark == 2) {
                lab->setText("O");
                lab->setStyleSheet("color:"+colO);
            } else {
                lab->clear();
            }
        }
}
