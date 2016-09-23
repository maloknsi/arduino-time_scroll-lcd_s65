#include "Arduino.h"
typedef uint8_t PROGMEM prog_uint8_t;
#define S65_LS020

#include "fonts.h"
#include "font_5x8.c"
#include "ls020.cpp"
#include "S65Display.cpp"

//======================================================================================================================
S65Display lcd;
//---------- Рассчетные константы отображения ---------
//позиция рамки по пикселям
const uint8_t grid_frame_x1 = 20;
const uint8_t grid_frame_x2 = 160;
const uint8_t grid_frame_y1 = 110;
const uint8_t grid_frame_y2 = 20;
const uint8_t grid_frame_width = grid_frame_x2 - grid_frame_x1; //160-20=140
const uint8_t grid_frame_height = grid_frame_y1 - grid_frame_y2; //110-20=90
//кол-во ячеек сетки в рамке
const uint8_t frame_cells_x = 7; //(160-20 /20)
const uint8_t frame_cells_y = 9; // (110-20 /10)

//---------- Переменные при инициализации -
//Максимальное значение по Y (mV), от него будет дальнейший расчет остальных переменных
uint16_t grid_max_value_y;

//---------- Рассчетные переменные --------
//кол-во значений на ячейку сетки по X,Y
uint16_t grid_cell_values_x, grid_cell_values_y;

//кол-во пикселей на ячейку сетки по X,Y
uint8_t grid_cell_pixels_x;
uint8_t grid_cell_pixels_y;

//кол-во значений на пиксель по X,Y
uint16_t grid_values_for_pixel_x;
uint16_t grid_values_for_pixel_y;

//счетчик сдвига рамки
uint8_t grid_frame_shift_pixels;

//Текущий X для вывода value - Y в рамке
uint8_t current_grid_pixel_x;

//счетчик позиции
uint16_t counter_value_x;

//Timer vars
//переменная инкремента таймера / 977 - 1секунда
volatile unsigned int timer_counter_seconds_for_pixel = 0;
//переменная инкремента таймера
volatile unsigned int timer_counter_one_second;
//флаг вызова функции прорисовки точки
volatile boolean timer_draw_grid_value = false;
//test seconds
volatile unsigned int timer_seconds;

//======================================================================================================================
// напечатать название позиции ячейки сетки
void draw_grid_cell_name_x(uint16_t grid_cell_name, uint8_t x) {
    //void S65Display::drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color)
    //uint8_t S65Display::drawText(uint8_t x, uint8_t y, char *s, uint8_t size, uint16_t color, uint16_t bg_color)
    //sprintf(tmp, "%01.2f", diagram_grid_cell_text);
    char tmp[5];
    uint8_t sec, min;
    if (grid_cell_name > 0) {
        sec = grid_cell_name % 60;
        min = grid_cell_name / 60;
        sprintf(tmp, "%02d", min);
        lcd.drawText(x - 11, grid_frame_y1 + 3, tmp, 1, RGB(255, 255, 200), RGB(0, 0, 0));
        sprintf(tmp, "%02d", sec);
        lcd.drawText(x - 11, grid_frame_y1 + 3 + 9, tmp, 1, RGB(255, 200, 255), RGB(0, 0, 0));
    }
}

// напечатать название значения ячейки сетки
void draw_grid_cell_name_y(uint16_t grid_cell_name, uint8_t y) {
    char tmp[5];
    float value;
    //нулевое значение не печатаем
    if (grid_cell_name > 0) {
        value = grid_cell_name;
        value = value / 1000;
        dtostrf(value, 1, 2, tmp);
        lcd.drawText(grid_frame_x1 - 20, y - 2, tmp, 1, RGB(255, 255, 255), RGB(0, 0, 0));
    }
}

// нарисовать сетку
void draw_grid() {
    uint8_t x, y;
    //значение названий ячеек сетки
    uint16_t grid_cell_name;
    //обнуляем счетчик позиции в сетке, так как перерисовываем сетку
    grid_frame_shift_pixels = 0;
    current_grid_pixel_x = 0;
    //---------- Вычисляем константы отображения ---------
    //Вычисляем кол-во пикселей на ячейку сетки значений
    grid_cell_pixels_y = grid_frame_height / frame_cells_y; //10
    //Вычисляем кол-во значений на ячейку сетки
    grid_cell_values_y = grid_max_value_y / frame_cells_y;
    //Вычисляем кол-во значений на точку
    grid_values_for_pixel_y = grid_cell_values_y / grid_cell_pixels_y;

    //Вычисляем кол-во пикселей на ячейку сетки позиций
    grid_cell_pixels_x = grid_frame_width / frame_cells_x; //10
    //Вычисляем кол-во позиций на ячейку сетки
    grid_cell_values_x = grid_values_for_pixel_x * grid_cell_pixels_x;

    //Начальные значения названий ячеек
    grid_cell_name = 0;
    // draw Y line
    for (x = grid_frame_x1; x <= grid_frame_x2; x = x + grid_cell_pixels_x) {
        lcd.drawLine(x, grid_frame_y1 + 8, x, grid_frame_y2, RGB(0, 150, 150));
        draw_grid_cell_name_x(grid_cell_name, x);
        grid_cell_name = grid_cell_name + grid_cell_values_x;
    }

    //Начальные значения названий ячеек
    grid_cell_name = counter_value_x;
    // draw X line
    for (y = grid_frame_y1; y >= grid_frame_y2; y = y - grid_cell_pixels_y) {
        lcd.drawLine(grid_frame_x1, y, grid_frame_x2, y, RGB(0, 150, 150));
        draw_grid_cell_name_y(grid_cell_name, y);
        grid_cell_name = grid_cell_name + grid_cell_values_y;
    }
}

// Нарисовать точку-значение на графике
void grid_clear_after_shift() {
    uint8_t x, y;
    //Если начался сдвиг - выполняем очистку
    if (grid_frame_shift_pixels > 0) {
        x = grid_frame_x1 + current_grid_pixel_x;

        //Очистка колонди сдвига
        lcd.drawLine(x, grid_frame_y1 + 3 + 9 + 8, x, grid_frame_y2, RGB(0, 0, 0));
        // прорисовка X-линий (продолжение точками)
        for (y = grid_frame_y1; y >= grid_frame_y2; y = y - grid_cell_pixels_y) {
            lcd.drawPixel(x, y, RGB(0, 150, 150));
        }
        //прорисовка Y-линий если значение по X = MOD cell
        if ((current_grid_pixel_x % grid_cell_pixels_x) == 0) {
            lcd.drawLine(x, grid_frame_y1 + 8, x, grid_frame_y2, RGB(0, 150, 150));
            //прорисовка названий по X-координатам
            if (current_grid_pixel_x > 0) {
                draw_grid_cell_name_x(counter_value_x - grid_values_for_pixel_x, x);
            } else {
                draw_grid_cell_name_x(counter_value_x - grid_values_for_pixel_x, grid_frame_x2);
            }
        }
    }
}

//Рассчитывает Y-пиксель значения в фрейме
uint8_t grid_calculate_pixel_y(uint16_t value) {
    //рассчитываем значение для сетки
    uint16_t grid_value = value / grid_values_for_pixel_y;
    return grid_frame_y1 - grid_value;
}

//Рассчитывает X-пиксель позиции  в фрейме, сдвигает график, если нужно
uint8_t grid_calculate_pixel_x() {
    //Если начался сдвиг - продолжаем сдвигать
    if (grid_frame_shift_pixels > 0) {
        current_grid_pixel_x = grid_frame_shift_pixels - 1;
        //делаем сдвиг
        lcd.scrollP(grid_frame_x1, (grid_frame_x2 - grid_frame_x1), grid_frame_shift_pixels);
        grid_frame_shift_pixels++;
        //если сдвиг больше ширины рамки - обнуляем сдвиг
        if (grid_frame_shift_pixels > grid_frame_width) {
            grid_frame_shift_pixels = 1;
            current_grid_pixel_x = grid_frame_width - 1;
        }

    } else {
        //иначе увеличиваем пиксель вывода
        current_grid_pixel_x++;
        //Если координата позиции больше рамки - начинаем сдвиг
        if (current_grid_pixel_x >= grid_frame_width) {
            grid_frame_shift_pixels = 1;
        }
    }
    counter_value_x+=grid_values_for_pixel_x;
    return grid_frame_x1 + current_grid_pixel_x;
}

// Нарисовать значение на графике
void draw_grid_value(uint16_t value) {
    //Рассчитывает X-пиксель позиции  в фрейме, сдвигает график, если нужно
    uint8_t grid_x = grid_calculate_pixel_x();
    //Рассчитывает Y-пиксель значения в фрейме
    uint8_t grid_y = grid_calculate_pixel_y(value);
    //рисуем новое значение
    grid_clear_after_shift();
    lcd.drawPixel(grid_x, grid_y, RGB(255, 150, 150));
}
//---------------------------------------------------------------------
SIGNAL(TIMER0_COMPA_vect)
{
    timer_counter_one_second++;
    // переполнение 977 - 976.5625 Гц - 1 секунда
    if (timer_counter_one_second >= 977){
        timer_seconds++;
        timer_counter_seconds_for_pixel++;
        timer_counter_one_second = 0;
        //если прошло кол-во позиций(секунд) на точку - разрешаем отрисовку точки в гриде
        if (timer_counter_seconds_for_pixel >= grid_values_for_pixel_x){
            timer_draw_grid_value = true;
            timer_counter_seconds_for_pixel = 0;
        }
    }
}
//---------------------------------------------------------------------

void setup() {
    Serial.begin(9600);      // open the serial port at 9600 bps:
    lcd.init(4); //spi-clk = Fcpu/4
    lcd.clear(RGB(0, 0, 0));
    // максимальное значение - mV, 9 Cells (value % 9)
    grid_max_value_y = 900;

    //кол-во позиций на точку (Time, sec)
    grid_values_for_pixel_x = 1;
    //обнуляем счетчик позиции
    counter_value_x = 0;
    draw_grid();
    //--------------------------------
//    uint16_t x;
//
//    for (x = 0; x < 100; x++) {
//        draw_grid_value(x * 10);
//    }
//    for (x = 100; x > 30; x--) {
//        draw_grid_value(x * 10);
//    }
//    timer_seconds = counter_position;
    //--------------------------------

    //Инициализация  Timer0 (уже используется millis())
    //прерываемся где-то посередине и вызываем ниже функцию "Compare A"
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);

}
void loop() {
    if (timer_draw_grid_value){
        timer_draw_grid_value = false;
        draw_grid_value(counter_value_x);
        Serial.print("timer_seconds = ");
        Serial.print(timer_seconds);
        Serial.print("; counter_position = ");
        Serial.print(counter_value_x);
        Serial.println();
    }
}
