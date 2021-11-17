/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
 TODO: Determine if the interrupt request bit can be manually set even if timer enable and/or timer int enable bits are 0.
*/

#include "pcfx.h"
#include "interrupt.h"
#include "timer.h"

#include "../state_helpers.h"

static uint16 control;
static uint16 period;
static int32 counter;

static int32 lastts;

static INLINE v810_timestamp_t CalcNextEventTS(const v810_timestamp_t timestamp)
{
   return((control & 0x2) ? (timestamp + counter) : PCFX_EVENT_NONONO);
}

#define EFF_PERIOD ((period ? period : 0x10000) * 15)

v810_timestamp_t FXTIMER_Update(const v810_timestamp_t timestamp)
{
   if(control & 0x2)
   {
      int32 cycles = timestamp - lastts;
      counter -= cycles;
      while(counter <= 0)
      {
         counter += EFF_PERIOD;
         if(control & 0x1)
         {
            control |= 0x4;
            PCFXIRQ_Assert(PCFXIRQ_SOURCE_TIMER, TRUE);
         }
      }
   }

   lastts = timestamp;

   return(CalcNextEventTS(timestamp));
}

void FXTIMER_ResetTS(int32 ts_base)
{
   lastts = ts_base;
}

uint16 FXTIMER_Read16(uint32 A, const v810_timestamp_t timestamp)
{
   FXTIMER_Update(timestamp);

   switch(A & 0xFC0)
   {
      default:
         break;
      case 0xF00:
         return(control);
      case 0xF80:
         return(period);
      case 0xFC0:
         return((counter + 14) / 15);
   }

   return 0;
}

uint8 FXTIMER_Read8(uint32 A, const v810_timestamp_t timestamp)
{
   FXTIMER_Update(timestamp);
   return(FXTIMER_Read16(A&~1, timestamp) >> ((A & 1) * 8));
}

void FXTIMER_Write16(uint32 A, uint16 V, const v810_timestamp_t timestamp)
{
   FXTIMER_Update(timestamp);

   switch(A & 0xFC0)
   {
      default:
         break;
      case 0xF00:
         if(!(control & 0x2) && (V & 0x2))
            counter = EFF_PERIOD;
         control = V & 0x7;

         PCFXIRQ_Assert(PCFXIRQ_SOURCE_TIMER, (bool)(control & 0x4));
         PCFX_SetEvent(PCFX_EVENT_TIMER, CalcNextEventTS(timestamp));
         break;

      case 0xF80:
         period = V; 
         PCFX_SetEvent(PCFX_EVENT_TIMER, CalcNextEventTS(timestamp));
         break;
   }
}

int FXTIMER_StateAction(StateMem *sm, int load, int data_only)
{
   SFORMAT StateRegs[] =
   {
      SFVAR(counter),
      SFVAR(period),
      SFVAR(control),
      SFEND
   };

   int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "TIMR", false);

   if(load)
   {

   }
   return(ret);
}

bool FXTIMER_GetRegister(const std::string &name, uint32 &value)
{
   if(name == "TCTRL")
   {
      value = control;
      return true;
   }
   else if(name == "TPRD")
   {
      value = period;
      return true;
   }
   else if(name == "TCNTR")
   {
      value = counter;
      return true;
   }
   return false;
}

void FXTIMER_Reset(void)
{
   control = 0;
   period = 0;
   counter = 0;
}

void FXTIMER_Init(void)
{
   lastts = 0;
   FXTIMER_Reset();
}
