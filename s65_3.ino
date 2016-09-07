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
const uint8_t grid_frame_width = grid_frame_x2 - grid_frame_x1;
const uint8_t grid_frame_height = grid_frame_y1 - grid_frame_y2;
//кол-во чеек сетки в рамке
const uint8_t frame_position_cells = 7;
const uint8_t frame_value_cells = 9;

//---------- Переменные при инициализации -
//Максимальное кол-во значений
uint16_t max_position, grid_max_value;

//---------- Рассчетные переменные --------
//кол-во пикселей на ячейку сетки
uint8_t grid_cell_position_pixels;
uint8_t grid_cell_value_pixels;

//кол-во позиций/значений на ячейку сетки
uint16_t grid_cell_positions, grid_cell_values;

//кол-во позиций/значений на точку
uint16_t grid_positions_for_pixel;
uint16_t grid_values_for_pixel;

//счетчик сдвига рамки
uint8_t grid_frame_shift_pixels;

//Текущий position - X для вывода value - Y
uint8_t current_grid_position_pixel;

//счетчик позиции
uint16_t counter_position;


//======================================================================================================================
// напечатать название позиции ячейки сетки
void draw_grid_cell_name_position(uint16_t grid_cell_name_position, uint8_t x) {
    //void S65Display::drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color)
    //uint8_t S65Display::drawText(uint8_t x, uint8_t y, char *s, uint8_t size, uint16_t color, uint16_t bg_color)
    //sprintf(tmp, "%01.2f", diagram_grid_cell_text);
    char tmp[5];
    uint8_t sec, min;
    if (grid_cell_name_position > 0) {
        sec = grid_cell_name_position % 60;
        min = grid_cell_name_position / 60;
        sprintf(tmp, "%02d", min);
        lcd.drawText(x - 11, grid_frame_y1 + 3, tmp, 1, RGB(255, 255, 200), RGB(0, 0, 0));
        //------- debug ------
        Serial.print(", time = ");
        Serial.print(tmp);
        //------- end debug
        sprintf(tmp, "%02d", sec);
        lcd.drawText(x - 11, grid_frame_y1 + 3 + 9, tmp, 1, RGB(255, 200, 255), RGB(0, 0, 0));
        //------- debug ------
        Serial.print(":");
        Serial.print(tmp);
        //------- end debug
    }

}

// напечатать название значения ячейки сетки
void draw_grid_cell_name_value(uint16_t grid_cell_name_value, uint8_t y) {
    char tmp[5];
    float value;
    if (grid_cell_name_value > 0) {
        value = grid_cell_name_value;
        value = value / 1000;
        dtostrf(value, 1, 2, tmp);
        lcd.drawText(grid_frame_x1 - 20, y - 2, tmp, 1, RGB(255, 255, 255), RGB(0, 0, 0));
    }
}

// нарисовать сетку
void draw_grid() {
    uint8_t x, y;
    //значение названий ячеек сетки
    uint16_t grid_cell_name_position, grid_cell_name_value;
    //обнуляем счетчик позиции в сетке, так как перерисовываем сетку
    grid_frame_shift_pixels = 0;
    current_grid_position_pixel = 0;
    //---------- Вычисляем константы отображения ---------
    //Вычисляем кол-во значений/пикселей на ячейку сетки
    grid_cell_values = grid_max_value / frame_value_cells;
    grid_cell_value_pixels = grid_frame_height / frame_value_cells;
    //Вычисляем кол-во позиций/пикселей на ячейку сетки
    grid_cell_position_pixels = grid_frame_width / frame_position_cells;

    //кол-во позиций на точку
    grid_positions_for_pixel = grid_cell_positions / grid_cell_position_pixels;
    grid_values_for_pixel = grid_max_value / grid_frame_height;

    //Начальные значения названий ячеек
    grid_cell_name_position = 0;
    grid_cell_name_value = 0;

    // draw Y line
    for (x = grid_frame_x1; x <= grid_frame_x2; x = x + grid_cell_position_pixels) {
        lcd.drawLine(x, grid_frame_y1 + 8, x, grid_frame_y2, RGB(0, 150, 150));
        draw_grid_cell_name_position(grid_cell_name_position, x);
        grid_cell_name_position = grid_cell_name_position + grid_cell_positions;
    }

    // draw X line
    for (y = grid_frame_y1; y >= grid_frame_y2; y = y - grid_cell_value_pixels) {
        lcd.drawLine(grid_frame_x1, y, grid_frame_x2, y, RGB(0, 150, 150));
        draw_grid_cell_name_value(grid_cell_name_value, y);
        grid_cell_name_value = grid_cell_name_value + grid_cell_values;
    }
}

// Нарисовать точку-значение на графике
void grid_clear_after_shift(){
    uint8_t x, y;
    //Если начался сдвиг - выполняем очистку
    if (grid_frame_shift_pixels > 0) {
        x = grid_frame_x1 + current_grid_position_pixel;
        //------- debug ------
        Serial.print("shift = ");
        Serial.print(grid_frame_shift_pixels);
        Serial.print(", position_pixel = ");
        Serial.print(current_grid_position_pixel);
        Serial.print(", x = ");
        Serial.print(x);
        //------- end debug

        //Очистка колонди сдвига
        lcd.drawLine(x, grid_frame_y1 + 3 + 9 +8, x, grid_frame_y2, RGB(0, 0, 0));
        //прорисовка линий
        // draw X line
        for (y = grid_frame_y1; y >= grid_frame_y2; y = y - grid_cell_value_pixels) {
            lcd.drawPixel(x, y, RGB(0, 150, 150));
        }
        //прорисовка Y линий
        if ((current_grid_position_pixel % grid_cell_position_pixels) == 0){
            lcd.drawLine(x, grid_frame_y1 + 8, x, grid_frame_y2, RGB(0, 150, 150));
            if (current_grid_position_pixel > 0){
                draw_grid_cell_name_position(counter_position - 1, x);
            } else {
                draw_grid_cell_name_position(counter_position - 1, grid_frame_x2);
            }
            //------- debug ------
            Serial.print(", draw name = ");
            Serial.print(counter_position - 1);
            //------- end debug
        }
        Serial.println();
    }
}

//Рассчитывает Y-пиксель значения в фрейме
uint8_t grid_calculate_pixel_value(uint16_t value){
    //рассчитываем значение для сетки
    uint16_t grid_value = value / grid_values_for_pixel;
    return grid_frame_y1 - grid_value;
}

//Рассчитывает X-пиксель позиции  в фрейме, сдвигает график, если нужно
uint8_t grid_calculate_pixel_position(){
    //Если начался сдвиг - продолжаем сдвигать
    if (grid_frame_shift_pixels > 0) {
        current_grid_position_pixel = grid_frame_shift_pixels - 1;
        //делаем сдвиг
        lcd.scrollP(grid_frame_x1,(grid_frame_x2 - grid_frame_x1),grid_frame_shift_pixels);
        grid_frame_shift_pixels++;
        //если сдвиг больше ширины рамки - обнуляем сдвиг
        if (grid_frame_shift_pixels > grid_frame_width) {
            grid_frame_shift_pixels = 1;
            current_grid_position_pixel = grid_frame_width - 1;
        }

    } else {
        //иначе увеличиваем пиксель вывода
        current_grid_position_pixel++;
        //Если координата позиции больше рамки - начинаем сдвиг
        if (current_grid_position_pixel >= grid_frame_width) {
            grid_frame_shift_pixels = 1;
        }
    }
    counter_position++;
    return grid_frame_x1 + current_grid_position_pixel;
}

// Нарисовать значение на графике
void draw_grid_value(uint16_t value) {
    //Рассчитывает X-пиксель позиции  в фрейме, сдвигает график, если нужно
    uint8_t grid_x = grid_calculate_pixel_position();
    //Рассчитывает Y-пиксель значения в фрейме
    uint8_t grid_y = grid_calculate_pixel_value(value);
    //рисуем новое значение
    grid_clear_after_shift();
    lcd.drawPixel(grid_x, grid_y, RGB(255, 150, 150));
}

void setup() {
    Serial.begin(9600);      // open the serial port at 9600 bps:
    lcd.init(4); //spi-clk = Fcpu/4
    lcd.clear(RGB(0, 0, 0));
    // максимальное значение - mV, 9 Cells (value % 9)
    grid_max_value = 1800;
    //кол-во позиций на ячейку сетки (Time, sec), 7 Cells
    grid_cell_positions = 20;
    //обнуляем счетчик позиции
    counter_position = 0;
    draw_grid();
}


void loop() {
    Serial.print('Test1');
    Serial.println('Test2');
    uint16_t x, direction = 10;
    uint8_t  delay_ = 1;
    for (x = 0; x < 100; x++) {
        draw_grid_value(x*10);
    }
    for (x = 100; x > 60; x--) {
        draw_grid_value(x*10);
    }
    x=600;

    while (true) {
        if (x > 1700) direction = -10;
        if (x < 100) direction = 10;
        x = x + direction;
        draw_grid_value(x);
        delay(delay_);
    }
}
