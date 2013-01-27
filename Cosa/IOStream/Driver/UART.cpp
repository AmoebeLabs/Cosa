/**
 * @file Cosa/IOStream/Driver/UART.cpp
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2012, Mikael Patel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 *
 * @section Description
 * Basic UART device handler with internal buffering.
 *
 * This file is part of the Arduino Che Cosa project.
 */

#include "Cosa/Bits.h"
#include "Cosa/IOStream/Driver/UART.hh"

UART uart;

bool
UART::begin(uint32_t baudrate, uint8_t format)
{
  uint16_t setting = (F_CPU / (baudrate * 8L)) - 1;

  // Check if double rate is not possible
  if (setting > 4095) {
    setting = (F_CPU / (baudrate * 16L)) - 1;
  } else {
    *UCSRnA() = _BV(U2X0);
  }

  // Set baudrate
  *UBRRn() = setting;

  // Enable transmitter interrupt
  *UCSRnB() = _BV(TXEN0);
    
  // Set frame format: asynchronous, 8data, 2stop bit
  *UCSRnC() = format;
  return (1);
}

bool 
UART::end()
{
  // Disable interrupts
  *UCSRnB() = 0;
  return (1);
}

int 
UART::putchar(char c)
{
  uint8_t next = (m_head + 1) & BUFFER_MASK;
  while (next == m_tail);
  m_buffer[next] = c;
  m_head = next;
  *UCSRnB() |= _BV(UDRIE0);
  return (c);
}

int 
UART::flush()
{
  uint32_t cycles = CYCLES_MAX;
  while (m_head != m_tail && cycles != 0) cycles--;
  return (cycles == 0 ? -1 : 0);
}

ISR(USART_UDRE_vect)
{
  if (uart.m_head != uart.m_tail) {
    uint8_t next = (uart.m_tail + 1) & UART::BUFFER_MASK;
    uart.m_tail = next;
    UDR0 = uart.m_buffer[next];
  }
  else {
    bit_clear(UCSR0B, UDRIE0);
  }
}

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#endif
