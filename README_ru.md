## 🌐 Доступные языки

- [English](README.md)
- [Русский](README_ru.md)

# Крестики-нолики

Игра в крестики-нолики с использованием микроконтроллера 80C31, смоделированного в Proteus, и Qt-приложения, обменивающихся данными через COM-порт.

---

## Как запустить

1. Клонируйте репозиторий:

    ```shell
    git clone https://github.com/bomburger/tic-tac-toe
    cd tic-tac-toe
    ```

2. Откройте проект Proteus: `Proteus/tictac.pdsprj`  
3. Создайте виртуальную пару COM-портов COM1 <=> COM2 (например, с помощью [VSPE](http://www.eterlogic.com/Products.VSPE.html)).  
4. Соберите и запустите Qt-приложение (через Qt Creator или `qmake`).  
5. Запустите симуляцию в Proteus.

---

## Как это работает

Проект состоит из двух программ:

- Одна работает на микроконтроллере в симуляции Proteus.
- Вторая — Qt-приложение.

Обе программы могут отображать и изменять состояние игры. Обмен данными происходит по COM-порту.

### Симуляция в Proteus

#### Аппаратная часть

1. Микроконтроллер 80C31  
2. 18 светодиодов (+ 6 NPN BJT транзисторов) для отображения поля 3×3  
3. 2 кнопки для выбора и размещения фигур  
4. 7-сегментный дисплей для отображения выбранной ячейки  
5. Светодиод, отображающий, чей сейчас ход

#### Программа

Код для микроконтроллера находится в `Proteus/src/main.c`.  
Сборка выполняется с помощью скрипта `build.bat` (требуется установленный `sdcc` в `PATH`):

```shell
cd Proteus
./build.bat
```

Этот скрипт создаёт файл `main.hex` в директории `Proteus/build`.  
Этот файл используется как прошивка микроконтроллера в симуляции.

### Qt-приложение

Проект Qt находится в файле `Qt/Project.pro`.  
Вы можете собрать его с помощью Qt Creator или вручную с помощью `qmake` и `make`.

### COM-порт

Чтобы программы могли обмениваться данными, создайте виртуальную пару COM-портов COM1 <=> COM2.  
Для этого можно использовать [VSPE](https://eterlogic.com/Products.VSPE.html).

Параметры COM-порта:

- Скорость (Baudrate): 9600  
- Биты данных: 8  
- Чётность (Parity): нет  
- Стоп-биты: 1

### Формат обмена сообщениями

#### Qt → МК

Для отправки сообщений из Qt-приложения в программу на микроконтроллере используется следующий формат:

1 байт: **rc0sdddd**

- `r` — установлен, если игра завершена и нужно сбросить состояние.
- `c` — установлен, если пользователь выбрал ячейку в Qt-приложении. В этом случае биты данных `d` указывают номер выбранной ячейки.
- Если и `r`, и `c` равны 0 — фигура была поставлена на поле.  
  `s` указывает, какая именно фигура (0 — X, 1 — O), `d` — номер ячейки.

#### МК → Qt

Для отправки сообщений от микроконтроллера в Qt-приложение используется следующий формат:

1 байт: **cs00dddd**

- `c` — установлен, когда нажата кнопка сдвига курсора.  
  При этом `d` показывает новую позицию курсора.
- `s` — установлен, когда нажата кнопка выбора (поместить фигуру).  
  В этом случае `d = 0`.
