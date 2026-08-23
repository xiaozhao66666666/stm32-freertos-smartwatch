#include "oled.h"
#include "font.h"

// 显示缓冲区
uint8_t OLED_GRAM[128][8] = {0};

// 内部函数声明
static void OLED_WriteByte(uint8_t data, uint8_t mode);
static void OLED_WriteCmd(uint8_t cmd);
static void OLED_WriteData(uint8_t data);

/**
  * @brief  写一个字节到OLED
  */
static void OLED_WriteByte(uint8_t data, uint8_t mode)
{
    uint8_t buffer[2];
    buffer[0] = mode;
    buffer[1] = data;
    
    HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDRESS, buffer, 2, 100);
}

/**
  * @brief  写命令
  */
static void OLED_WriteCmd(uint8_t cmd)
{
    OLED_WriteByte(cmd, OLED_CMD);
}

/**
  * @brief  写数据
  */
static void OLED_WriteData(uint8_t data)
{
    OLED_WriteByte(data, OLED_DATA);
}

/**
  * @brief  OLED初始化
  */
void OLED_Init(void)
{
    // 等待OLED上电稳定
    HAL_Delay(100);
    
    // 初始化序列
    OLED_WriteCmd(0xAE); // 关闭显示
    OLED_WriteCmd(0x20); // 设置内存地址模式
    OLED_WriteCmd(0x10); // 00:水平地址模式 01:垂直地址模式 10:页地址模式
    OLED_WriteCmd(0xB0); // 设置页起始地址
    OLED_WriteCmd(0xC8); // 设置扫描方向
    OLED_WriteCmd(0x00); // 设置列地址低位
    OLED_WriteCmd(0x10); // 设置列地址高位
    OLED_WriteCmd(0x40); // 设置显示起始行
    OLED_WriteCmd(0x81); // 对比度设置
    OLED_WriteCmd(0xFF); // 对比度值: 0~255
    OLED_WriteCmd(0xA1); // 设置段重定向
    OLED_WriteCmd(0xA6); // 正常显示
    OLED_WriteCmd(0xA8); // 多路复用率
    OLED_WriteCmd(0x3F); // 1/64 duty
    OLED_WriteCmd(0xD3); // 设置显示偏移
    OLED_WriteCmd(0x00); // 无偏移
    OLED_WriteCmd(0xD5); // 设置振荡器频率
    OLED_WriteCmd(0x80); // 
    OLED_WriteCmd(0xD9); // 设置预充电周期
    OLED_WriteCmd(0xF1); // 
    OLED_WriteCmd(0xDA); // 设置COM引脚硬件配置
    OLED_WriteCmd(0x12); // 
    OLED_WriteCmd(0xDB); // 设置VCOMH
    OLED_WriteCmd(0x40); // 
    OLED_WriteCmd(0x8D); // 电荷泵设置
    OLED_WriteCmd(0x14); // 启用电荷泵
    OLED_WriteCmd(0xAF); // 打开OLED显示
    
    // 清屏
    OLED_Clear();
    OLED_Refresh();
}

/**
  * @brief  清屏
  */
void OLED_Clear(void)
{
    for(uint8_t i = 0; i < 8; i++)
    {
        for(uint8_t j = 0; j < 128; j++)
        {
            OLED_GRAM[j][i] = 0;
        }
    }
}

/**
  * @brief  刷新屏幕
  */
void OLED_Refresh(void)
{
    for(uint8_t i = 0; i < 8; i++)
    {
        OLED_WriteCmd(0xB0 + i); // 设置页地址
        OLED_WriteCmd(0x00);     // 设置列地址低位
        OLED_WriteCmd(0x10);     // 设置列地址高位
        
        for(uint8_t j = 0; j < 128; j++)
        {
            OLED_WriteData(OLED_GRAM[j][i]);
        }
    }
}

/**
  * @brief  设置光标位置
  */
void OLED_SetCursor(uint8_t page, uint8_t col)
{
    OLED_WriteCmd(0xB0 + page);          // 设置页地址
    OLED_WriteCmd(((col & 0xF0) >> 4) | 0x10); // 设置列地址高位
    OLED_WriteCmd(col & 0x0F);           // 设置列地址低位
}

/**
  * @brief  显示一个字符
  */
void OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t size)
{
    uint8_t temp, pos;
    uint8_t *pfont = 0;
    
    if(size == FONT_SIZE_6x8)
    {
        if(ch < ' ' || ch > '~') return;
        pfont = (uint8_t *)&ASCII_6x8[(ch - ' ') * 6];
        
        for(uint8_t i = 0; i < 6; i++)
        {
            temp = pfont[i];
            for(uint8_t j = 0; j < 8; j++)
            {
                if(temp & 0x01)
                    OLED_DrawPoint(x + i, y + j, WHITE);
                else
                    OLED_DrawPoint(x + i, y + j, BLACK);
                temp >>= 1;
            }
        }
    }
    else if(size == FONT_SIZE_8x16)
    {
        if(ch < ' ' || ch > '~') return;
        pfont = (uint8_t *)&ASCII_8x16[(ch - ' ') * 16];
        
        for(uint8_t i = 0; i < 16; i++)
        {
            temp = pfont[i];
            for(uint8_t j = 0; j < 8; j++)
            {
                if(temp & 0x01)
                    OLED_DrawPoint(x + j, y + i, WHITE);
                else
                    OLED_DrawPoint(x + j, y + i, BLACK);
                temp >>= 1;
            }
        }
    }
}

/**
  * @brief  显示字符串
  */
void OLED_ShowString(uint8_t x, uint8_t y, char *str, uint8_t size)
{
    while(*str)
    {
        OLED_ShowChar(x, y, *str, size);
        if(size == FONT_SIZE_6x8)
            x += 6;
        else if(size == FONT_SIZE_8x16)
            x += 8;
        str++;
    }
}

/**
  * @brief  显示数字
  */
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size)
{
    char str[20];
    sprintf(str, "%d", num);
    OLED_ShowString(x, y, str, size);
}

/**
  * @brief  显示浮点数
  */
void OLED_ShowFloatNum(uint8_t x, uint8_t y, float num, uint8_t int_len, uint8_t size)
{
    char str[20];
    sprintf(str, "%.2f", num);
    OLED_ShowString(x, y, str, size);
}

/**
  * @brief  显示中文字符
  */
void OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t index)
{
    uint8_t i, j;
    const uint8_t *p = &Chinese_16x16[index * 32];
    
    for(i = 0; i < 16; i++)
    {
        for(j = 0; j < 8; j++)
        {
            if(p[i * 2] & (0x80 >> j))
                OLED_DrawPoint(x + j, y + i, WHITE);
            else
                OLED_DrawPoint(x + j, y + i, BLACK);
            
            if(p[i * 2 + 1] & (0x80 >> j))
                OLED_DrawPoint(x + j + 8, y + i, WHITE);
            else
                OLED_DrawPoint(x + j + 8, y + i, BLACK);
        }
    }
}

/**
  * @brief  画点
  */
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t color)
{
    if(x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
    
    uint8_t page = y / 8;
    uint8_t bit = y % 8;
    
    if(color)
        OLED_GRAM[x][page] |= (1 << bit);
    else
        OLED_GRAM[x][page] &= ~(1 << bit);
}

/**
  * @brief  画线
  */
void OLED_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color)
{
    int16_t dx = abs(x2 - x1);
    int16_t dy = abs(y2 - y1);
    int16_t sx = (x1 < x2) ? 1 : -1;
    int16_t sy = (y1 < y2) ? 1 : -1;
    int16_t err = dx - dy;
    
    while(1)
    {
        OLED_DrawPoint(x1, y1, color);
        if(x1 == x2 && y1 == y2) break;
        
        int16_t e2 = 2 * err;
        if(e2 > -dy)
        {
            err -= dy;
            x1 += sx;
        }
        if(e2 < dx)
        {
            err += dx;
            y1 += sy;
        }
    }
}

/**
  * @brief  画矩形
  */
void OLED_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color)
{
    OLED_DrawLine(x1, y1, x2, y1, color);
    OLED_DrawLine(x1, y2, x2, y2, color);
    OLED_DrawLine(x1, y1, x1, y2, color);
    OLED_DrawLine(x2, y1, x2, y2, color);
}

/**
  * @brief  填充矩形
  */
void OLED_Fill(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color)
{
    for(uint8_t i = y1; i <= y2; i++)
    {
        for(uint8_t j = x1; j <= x2; j++)
        {
            OLED_DrawPoint(j, i, color);
        }
    }
}

/**
  * @brief  显示位图
  */
void OLED_ShowBMP(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bmp)
{
    for(uint8_t i = 0; i < height / 8; i++)
    {
        for(uint8_t j = 0; j < width; j++)
        {
            OLED_GRAM[x + j][y + i] = bmp[i * width + j];
        }
    }
}