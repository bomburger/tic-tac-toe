#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <array>
#include <QPushButton>
#include <QSerialPort>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_btnInc_clicked();              // «+»
    void on_btnOk_clicked();               // «OK»
    void on_cell_clicked();                // любой из 9 <QPushButton>
    void on_data_recieved();                // recieve data from com port

private:
    Ui::MainWindow *ui;
    QSerialPort *port;

    std::array<int,9> board{};             // 0-пусто,1-X,2-O
    int cursor = 0;                        // выбранная клетка (0-8)
    int player = 1;                        // чей ход: 1-X,2-O

    std::array<QPushButton*,9> cells;      // указатели на 9 кнопок
    void updateHighlight();                // меняем рамку активной

    void updateCursorLCD();                // вывести cursor на LCD
    void redrawBoard();                    // перерисовать X/O/-
    void switchPlayer();                   // X⇄O
    bool checkWin(int p) const;            // победа?
    void resetGame();                      // очистить поле
    void initPort();
    void sendMoveData(int shape, int cell);
};
#endif
