#include <Servo.h>
#include "config.h"
#include "Stepper.hpp"
#include "Draw2D.hpp"

#define LOG(msg) Serial.print(msg)
#define LOGLN(msg) Serial.println(msg)

#define OP_MOVE 0
#define OP_LINE 1
#define OP_MOVE_REL 2
#define OP_LINE_REL 3

#define LINE_BUFFER_SIZE 100
#define RX_BUFFER_SIZE 50000
#define STEPS_PER_MILLIMETER 26

struct header
{
	uint16_t instruction_count;
	//...
} __attribute__((packed));

struct instruction
  {
	int op;
	float x;
	float y;
  };

void lowerPen(bool);

#define PEN_PIN D4 //D10
#define PEN_DRAWING 120
#define PEN_MOVING 100
bool penStatus; //true if pen is up
Servo pen;

Stepper x(D1, D2);
Stepper y(D5, D6);
Draw2D draw(x, y, 0, 0, lowerPen);

struct instruction *instructions = NULL;
uint16_t instruction_index;
uint16_t instruction_count = 0;

#define PEN_UP true
#define PEN_DOWN false

//Raise (false) or lower (true) the pen
void lowerPen(bool status)
  {
	if(status != penStatus)
	  {
		if(status) //raising pen
		  {
			pen.write(PEN_MOVING);
			delay(50); //don't move anything until pen is up
		  }
		else //lowering pen
		  {
			uint8_t angle = PEN_MOVING;
			while(angle < PEN_DRAWING)
			  {
				angle += 2;
				pen.write(angle);
				delay(30);
			  }
		  }
		penStatus = status;
	  }
  }

void setup()
  {
  Serial.setRxBufferSize(RX_BUFFER_SIZE);
	Serial.begin(115200);

	LOGLN("Initializing servo");
	pen.attach(PEN_PIN, 1000, 2000);
	pen.write(PEN_MOVING);
	penStatus = PEN_UP;

	LOGLN("Initializing steppers");
	x.init();
	x.setSpeed(100);
	y.init();
	y.setSpeed(100);
	delay(1000);
	lowerPen(PEN_DOWN);
	delay(1000);
	lowerPen(PEN_UP);
}

//line looks like "G1 X119.448 Y15.515 E1.48456"
void parse(instruction* ins, char* line)
  {
  char* token=strtok(line," ");
  while (token!=NULL)
    {
    // LOG(token);
    // LOG(" is ");
    // LOGLN(&token[1]);

    if (token[0]=='G')
      {
      ins->op=atoi(&token[1]);
      }
    else if (token[0]=='X')
      {
      ins->x=atof(&token[1]);
      ins->x=ins->x*STEPS_PER_MILLIMETER;
      }
    else if (token[0]=='Y')
      {
      ins->y=atof(&token[1]);
      ins->y=ins->y*STEPS_PER_MILLIMETER;
    }
    token = strtok(NULL, " ");  
    }
  }

void action(char* line)
  {
  instruction ins;
  ins.x=-1;
  ins.y=-1;
  parse(&ins,line);

  // LOG("Instruction: op=");
  // LOG(ins.op);
  // LOG(" x=");
  // LOG(ins.x);
  // LOG(" y=");
  // LOGLN(ins.y);

  if (ins.x>=0 && ins.y>=0)
    {
    if (ins.x>MAX_X)
      {
      LOG("Limiting X value to ");
      LOGLN(MAX_X);
      ins.x=MAX_X;
      }

    if (ins.y>MAX_Y)
      {
      LOG("Limiting Y value to ");
      LOGLN(MAX_Y);
      ins.y=MAX_Y;
      }

    switch(ins.op)
      {
        case OP_MOVE:  //0
          LOG("Moving to ");
          LOG(ins.x);
          LOG(", ");
          LOGLN(ins.y);
          draw.moveTo(ins.x, ins.y);
          break;

        case OP_LINE: //1 
          LOG("Line to ");
          LOG(ins.x);
          LOG(", ");
          LOGLN(ins.y);
          draw.lineTo(ins.x, ins.y);
          break;

        case OP_MOVE_REL:  //2
          LOG("Relative move to ");
          LOG(ins.x);
          LOG(", ");
          LOGLN(ins.y);
          draw.move(ins.x, ins.y);
          break;

        case OP_LINE_REL: //3
          LOG("Relative line to ");
          LOG(ins.x);
          LOG(", ");
          LOGLN(ins.y);
          draw.line(ins.x, ins.y);
          break;
      }
    }
  // String retval="Instruction:\nop=";
  // retval+=String(ins.op);
  // retval+=" x=";
  // retval+=String(ins.x);
  // retval+=" y=";
  // retval+=String(ins.y);
  // retval+=" pen=";
  // retval+=penStatus?"UP":"DOWN";
  // return retval;
	}


void loop()
  {
  int input=Serial.available();
  static int emptyLineCount=0;
	if(input)
	  {
    char inLine[LINE_BUFFER_SIZE];
		int count=Serial.readBytesUntil('\n',inLine,LINE_BUFFER_SIZE);
    LOG("Read ");
    LOG(count);
    LOGLN(" bytes.");
    inLine[count]='\0';
    if (count>0)
		  {
      if (inLine[0]=='G' && inLine[1]>='0' && inLine[1]<='9')
        {
        LOG("Received gcode: ");
        LOGLN(inLine);
        action(inLine);
//        Serial.println();
        }
      else
        {
        LOG("Received cruft: ");
        LOGLN(inLine);
        return;
        }
		  }
    else
      {
      if (emptyLineCount++>10)
        {
        emptyLineCount=0;
        LOGLN("Done.");
        }
      }
	  }
  }

