// TinyBASIC.cpp : Defines the entry point for the console application.
// Author: Mike Field <hamster@snap.net.nz>
//
// Made to fit on ATTiny w/OLED by Steve.
// This is the definition of "just because you can, doesn't mean you should."
//
#include <DigisparkOLED.h>
#define progver "0.02"
#define LED 1
#define BUT1 0
#define BUT2 2

// v0.02b: 2012-09-17  Scott Lawrence <yorgle@gmail.com>
//         Better FILES listings
//
// v0.02a: 2012-09-17  Scott Lawrence <yorgle@gmail.com>
//         Support for SD Library
//         Added: SAVE, FILES (mostly works), LOAD (mostly works) (redirects IO)
//         Added: MEM, ? (PRINT)
//         Quirk:  "10 LET A=B+C" is ok "10 LET A = B + C" is not.
//         Quirk:  INPUT seems broken?

// 4080 bytes on attiny - needs to be <4096 for ATTINY45 compatibility
// Global variables use 512 bytes - bring to 512


// IF testing with Visual C 
//#include "stdafx.h"
//char eliminateCompileErrors = 1; 

#define ARDUINO 1
#define FZ 6
#define FH 8


// Feature configuration...

// This enables LOAD, SAVE, FILES commands through the Arduino SD Library
// it adds 9k of usage as well.
//#define ENABLE_FILEIO 0
#undef ENABLE_FILEIO

//#define kRamSize   113 // ATTINY45
#define kRamSize 232 // ATTINY85

// ASCII Characters
#define CR  '\r'
#define NL  '\n'
#define TAB '\t'
#define BELL  '\b'
#define SPACE   ' '
#define SQUOTE  '\''
#define DQUOTE  '\"'
//#define CTRLC 0x03
//#define CTRLH 0x08
//#define CTRLS 0x13
//#define CTRLX 0x18

typedef short unsigned LINENUM;
#if ARDUINO
  #define ECHO_CHARS 1
#else
  #define ECHO_CHARS 0
#endif



static unsigned char program[kRamSize];
static unsigned char *txtpos,*list_line;
static unsigned char expression_error;
static unsigned char *tempsp;

int cursorX = 0;
int cursorY = 0;
int checkChar = 0;
int xres = 20;
int yres = (64/FH) - 1;
int row = 0;
int cpos = 0;
int ppos = 0;

/***********************************************************/
// Keyword table and constants - the last character has 0x80 added to it
static const unsigned char keywords[] = {
  'L','I','S','T'+0x80,
  'R','U','N'+0x80,
  'N','E','X','T'+0x80,
  'L','E','T'+0x80,
  'I','F'+0x80,
  'G','O','T','O'+0x80,
  'G','O','S','U','B'+0x80,
  'R','E','T','U','R','N'+0x80,
  'F','O','R'+0x80,
  'I','N','P','U','T'+0x80,
  'P','R','I','N','T'+0x80,
  'M','E','M'+0x80,
  '?'+ 0x80,
  0
};

#define KW_LIST   0
#define KW_RUN    1
#define KW_NEXT   2
#define KW_LET    3
#define KW_IF   4
#define KW_GOTO   5
#define KW_GOSUB  6
#define KW_RETURN 7
#define KW_FOR    8
#define KW_INPUT  9
#define KW_PRINT  10
#define KW_MEM          11
#define KW_DEFAULT  12

struct stack_for_frame {
  char frame_type;
  char for_var;
  short int terminal;
  short int step;
  unsigned char *current_line;
  unsigned char *txtpos;
};

struct stack_gosub_frame {
  char frame_type;
  unsigned char *current_line;
  unsigned char *txtpos;
};

/*static unsigned char func_tab[] = {
  'A','B','S'+0x80,
  0
};
#define FUNC_ABS   0
#define FUNC_UNKNOWN 1*/

static unsigned char to_tab[] = {
  'T','O'+0x80,
  0
};

static unsigned char step_tab[] = {
  'S','T','E','P'+0x80,
  0
};

static unsigned char relop_tab[] = {
  '>','='+0x80,
  '<','>'+0x80,
  '>'+0x80,
  '='+0x80,
  '<','='+0x80,
  '<'+0x80,
  0
};

#define RELOP_GE    0
#define RELOP_NE    1
#define RELOP_GT    2
#define RELOP_EQ    3
#define RELOP_LE    4
#define RELOP_LT    5
#define RELOP_UNKNOWN 6

#define STACK_SIZE (sizeof(struct stack_for_frame)*5)
#define VAR_SIZE sizeof(short int) // Size of variables in bytes

static unsigned char *stack_limit;
static unsigned char *program_start;
static unsigned char *program_end;
static unsigned char *stack; // Software stack for things that should go on the CPU stack
static unsigned char *variables_begin;
static unsigned char *current_line;
static unsigned char *sp;
#define STACK_GOSUB_FLAG 'G'
#define STACK_FOR_FLAG 'F'
static unsigned char table_index;
static LINENUM linenum;

static const unsigned char okmsg[]            = "READY";
static const unsigned char errmsg[]          = "Err";
static const unsigned char initmsg[]          = "ATTinyBasic " progver;
static const unsigned char memorymsg[]        = " Bytes Free.";

static int inchar(void);
static void outchar(unsigned char c);
static void line_terminator(void);
static short int expression(void);
static unsigned char breakcheck(void);

int pressed = 0;
/***************************************************************************/
static void ignore_blanks(void)
{
  while(*txtpos == SPACE || *txtpos == TAB)
    txtpos++;
}


/***************************************************************************/
static void scantable(const unsigned char *table)
{
  int i = 0;
  table_index = 0;
  while(1)
  {
    // Run out of table entries?
    if(table[0] == 0)
            return;

    // Do we match this character?
    if(txtpos[i] == table[0])
    {
      i++;
      table++;
    }
    else
    {
      // do we match the last character of keywork (with 0x80 added)? If so, return
      if(txtpos[i]+0x80 == table[0])
      {
        txtpos += i+1;  // Advance the pointer to following the keyword
        ignore_blanks();
        return;
      }

      // Forward to the end of this keyword
      while((table[0] & 0x80) == 0)
        table++;

      // Now move on to the first character of the next word, and reset the position index
      table++;
      table_index++;
      ignore_blanks();
      i = 0;
    }
  }
}

/***************************************************************************/
static void pushb(unsigned char b)
{
  sp--;
  *sp = b;
}

/***************************************************************************/
static unsigned char popb()
{
  unsigned char b;
  b = *sp;
  sp++;
  return b;
}

/***************************************************************************/
static void printnum(int num)
{
  int digits = 0;

  if(num < 0)
  {
    num = -num;
    outchar('-');
  }

  do {
    pushb(num%10+'0');
    num = num/10;
    digits++;
  }
  while (num > 0);

  while(digits > 0)
  {
    outchar(popb());
    digits--;
  }
}
/***************************************************************************/
static unsigned short testnum(void)
{
  unsigned short num = 0;
  ignore_blanks();
  
  while(*txtpos>= '0' && *txtpos <= '9' )
  {
    // Trap overflows
    if(num >= 0xFFFF/10)
    {
      num = 0xFFFF;
      break;
    }

    num = num *10 + *txtpos - '0';
    txtpos++;
  }
  return  num;
}

/***************************************************************************/
static void printmsgNoNL(const unsigned char *msg)
{
  while(*msg)
  {
    outchar(*msg);
    msg++;
  }
}

/***************************************************************************/
static unsigned char print_quoted_string(void)
{
  int i=0;
  unsigned char delim = *txtpos;
  if(delim != '"' && delim != '\'')
    return 0;
  txtpos++;

  // Check we have a closing delimiter
  while(txtpos[i] != delim)
  {
    if(txtpos[i] == NL)
      return 0;
    i++;
  }

  // Print the characters
  while(*txtpos != delim)
  {
    outchar(*txtpos);
    txtpos++;
  }
  txtpos++; // Skip over the last delimiter

  return 1;
}

/***************************************************************************/
static void printmsg(const unsigned char *msg)
{
  printmsgNoNL(msg);
    line_terminator();
}

/***************************************************************************/
static void getln(char prompt)
{
  outchar(prompt);
  txtpos = program_end+sizeof(LINENUM);

  while(1)
  {
    char c = inchar();
    switch(c)
    {
      case NL:
        break;
      case CR:
                                line_terminator();
        // Terminate all strings with a NL
        txtpos[0] = NL;
        return;
/*      case CTRLH:
        if(txtpos == program_end)
          break;
        txtpos--;
        printmsg(backspacemsg);
        break;*/
      default:
        // We need to leave at least one space to allow us to shuffle the line into order
        if(txtpos == variables_begin-2)
          outchar(BELL);
        else
        {
          txtpos[0] = c;
          txtpos++;
#if ECHO_CHARS
          outchar(c);
#endif
        }
    }
  }
}

/***************************************************************************/
static unsigned char *findline(void)
{
  unsigned char *line = program_start;
  while(1)
  {
    if(line == program_end)
      return line;

    if(((LINENUM *)line)[0] >= linenum)
      return line;

    // Add the line length onto the current address, to get to the next line;
    line += line[sizeof(LINENUM)];
  }
}

/***************************************************************************/
static void toUppercaseBuffer(void)
{
  unsigned char *c = program_end+sizeof(LINENUM);
  unsigned char quote = 0;

  while(*c != NL)
  {
    // Are we in a quoted string?
    if(*c == quote)
      quote = 0;
    else if(*c == '"' || *c == '\'')
      quote = *c;
    else if(quote == 0 && *c >= 'a' && *c <= 'z')
      *c = *c + 'A' - 'a';
    c++;
  }
}

/***************************************************************************/
void printline()
{
  LINENUM line_num;
  
  line_num = *((LINENUM *)(list_line));
    list_line += sizeof(LINENUM) + sizeof(char);

  // Output the line */
  printnum(line_num);
  outchar(' ');
  while(*list_line != NL)
    {
    outchar(*list_line);
    list_line++;
  }
  list_line++;
  line_terminator();
}

/***************************************************************************/
static short int expr4(void)
{
  if(*txtpos == '0')
  {
    txtpos++;
    return 0;
  }

  if(*txtpos >= '1' && *txtpos <= '9')
  {
    short int a = 0;
    do  {
      a = a*10 + *txtpos - '0';
      txtpos++;
    } while(*txtpos >= '0' && *txtpos <= '9');
    return a;
  }

  // Is it a function or variable reference?
  if(txtpos[0] >= 'A' && txtpos[0] <= 'Z')
  {
    short int a;
    // Is it a variable reference (single alpha)
    if(txtpos[1] < 'A' || txtpos[1] > 'Z')
    {
      a = ((short int *)variables_begin)[*txtpos - 'A'];
      txtpos++;
      return a;
    }

    // Is it a function with a single parameter
    //scantable(func_tab);
    //if(table_index == FUNC_UNKNOWN)
    //  goto expr4_error;

    //unsigned char f = table_index;

    //if(*txtpos != '(')
    //  goto expr4_error;

    //txtpos++;
    //a = expression();
    //if(*txtpos != ')')
    //    goto expr4_error;
    txtpos++;
    /*switch(table_index)
    {
      case FUNC_ABS:
        if(a < 0) 
          return -a;
        return a;
    }*/
  }

  if(*txtpos == '(')
  {
    short int a;
    txtpos++;
    a = expression();
    if(*txtpos != ')')
      goto expr4_error;

    txtpos++;
    return a;
  }

expr4_error:
  expression_error = 1;
  return 0;

}

/***************************************************************************/
static short int expr3(void)
{
  short int a,b;

  a = expr4();
  while(1)
  {
    if(*txtpos == '*')
    {
      txtpos++;
      b = expr4();
      a *= b;
    }
    else if(*txtpos == '/')
    {
      txtpos++;
      b = expr4();
      if(b != 0)
        a /= b;
      else
        expression_error = 1;
    }
    else
      return a;
  }
}

/***************************************************************************/
static short int expr2(void)
{
  short int a,b;

  if(*txtpos == '-' || *txtpos == '+')
    a = 0;
  else
    a = expr3();

  while(1)
  {
    if(*txtpos == '-')
    {
      txtpos++;
      b = expr3();
      a -= b;
    }
    else if(*txtpos == '+')
    {
      txtpos++;
      b = expr3();
      a += b;
    }
    else
      return a;
  }
}
/***************************************************************************/
static short int expression(void)
{
  short int a,b;

  a = expr2();
  // Check if we have an error
  if(expression_error)  return a;

  scantable(relop_tab);
  if(table_index == RELOP_UNKNOWN)
    return a;
  
  switch(table_index)
  {
  case RELOP_GE:
    b = expr2();
    if(a >= b) return 1;
    break;
  case RELOP_NE:
    b = expr2();
    if(a != b) return 1;
    break;
  case RELOP_GT:
    b = expr2();
    if(a > b) return 1;
    break;
  case RELOP_EQ:
    b = expr2();
    if(a == b) return 1;
    break;
  case RELOP_LE:
    b = expr2();
    if(a <= b) return 1;
    break;
  case RELOP_LT:
    b = expr2();
    if(a < b) return 1;
    break;
  }
  return 0;
}

/***************************************************************************/
void loop()
{
  unsigned char *start;
  unsigned char *newEnd;
  unsigned char linelen;
  
  program_start = program;
  program_end = program_start;
  sp = program+sizeof(program);  // Needed for printnum
  stack_limit = program+sizeof(program)-STACK_SIZE;
  variables_begin = stack_limit - 27*VAR_SIZE;
  printmsg(initmsg);
  printnum(variables_begin-program_end);
  printmsg(memorymsg);

warmstart:
  // this signifies that it is running in 'direct' mode.
  current_line = 0;
  sp = program+sizeof(program);  
  printmsg(okmsg);

prompt:
  getln('>');
  toUppercaseBuffer();

  txtpos = program_end+sizeof(unsigned short);

  // Find the end of the freshly entered line
  while(*txtpos != NL)
    txtpos++;

  // Move it to the end of program_memory
  {
    unsigned char *dest;
    dest = variables_begin-1;
    while(1)
    {
      *dest = *txtpos;
      if(txtpos == program_end+sizeof(unsigned short))
        break;
      dest--;
      txtpos--;
    }
    txtpos = dest;
  }

  // Now see if we have a line number
  linenum = testnum();
  ignore_blanks();
  if(linenum == 0)
    goto direct;

  if(linenum == 0xFFFF)
    goto qhow;

  // Find the length of what is left, including the (yet-to-be-populated) line header
  linelen = 0;
  while(txtpos[linelen] != NL)
    linelen++;
  linelen++; // Include the NL in the line length
  linelen += sizeof(unsigned short)+sizeof(char); // Add space for the line number and line length

  // Now we have the number, add the line header.
  txtpos -= 3;
  *((unsigned short *)txtpos) = linenum;
  txtpos[sizeof(LINENUM)] = linelen;


  // Merge it into the rest of the program
  start = findline();

  // If a line with that number exists, then remove it
  if(start != program_end && *((LINENUM *)start) == linenum)
  {
    unsigned char *dest, *from;
    unsigned tomove;

    from = start + start[sizeof(LINENUM)];
    dest = start;

    tomove = program_end - from;
    while( tomove > 0)
    {
      *dest = *from;
      from++;
      dest++;
      tomove--;
    } 
    program_end = dest;
  }

  if(txtpos[sizeof(LINENUM)+sizeof(char)] == NL) // If the line has no txt, it was just a delete
    goto prompt;



  // Make room for the new line, either all in one hit or lots of little shuffles
  while(linelen > 0)
  { 
    unsigned int tomove;
    unsigned char *from,*dest;
    unsigned int space_to_make;
  
    space_to_make = txtpos - program_end;

    if(space_to_make > linelen)
      space_to_make = linelen;
    newEnd = program_end+space_to_make;
    tomove = program_end - start;


    // Source and destination - as these areas may overlap we need to move bottom up
    from = program_end;
    dest = newEnd;
    while(tomove > 0)
    {
      from--;
      dest--;
      *dest = *from;
      tomove--;
    }

    // Copy over the bytes into the new space
    for(tomove = 0; tomove < space_to_make; tomove++)
    {
      *start = *txtpos;
      txtpos++;
      start++;
      linelen--;
    }
    program_end = newEnd;
  }
  goto prompt;

unimplemented:
  printmsg(errmsg);
  goto prompt;

qhow: 
  printmsg(errmsg);
  goto prompt;

qwhat:  
  printmsgNoNL(errmsg);
  if(current_line != NULL)
  {
           unsigned char tmp = *txtpos;
       if(*txtpos != NL)
        *txtpos = '^';
           list_line = current_line;
           printline();
           *txtpos = tmp;
  }
    line_terminator();
  goto prompt;

qsorry: 
  printmsg(errmsg);
  goto warmstart;

run_next_statement:
  while(*txtpos == ':')
    txtpos++;
  ignore_blanks();
  if(*txtpos == NL)
    goto execnextline;
  goto interperateAtTxtpos;

direct: 
  txtpos = program_end+sizeof(LINENUM);
  if(*txtpos == NL)
    goto prompt;

interperateAtTxtpos:
        /*if(breakcheck())
        {
          printmsg(breakmsg);
          goto warmstart;
        }*/

  scantable(keywords);

  switch(table_index)
  {
    case KW_LIST:
      goto list;
    case KW_MEM:
      goto mem;
    case KW_RUN:
      current_line = program_start;
      goto execline;
    case KW_NEXT:
      goto next;
    case KW_LET:
      goto assignment;
    case KW_IF:
      short int val;
      expression_error = 0;
      val = expression();
      if(expression_error || *txtpos == NL)
        goto qhow;
      if(val != 0)
        goto interperateAtTxtpos;
      goto execnextline;

    case KW_GOTO:
      expression_error = 0;
      linenum = expression();
      if(expression_error || *txtpos != NL)
        goto qhow;
      current_line = findline();
      goto execline;

    case KW_GOSUB:
      goto gosub;
    case KW_RETURN:
      goto gosub_return; 
    case KW_FOR:
      goto forloop; 
    case KW_INPUT:
      goto input; 
    case KW_PRINT:
      goto print;
    case KW_DEFAULT:
      goto assignment;
    default:
      break;
  }
  
execnextline:
  if(current_line == NULL)    // Processing direct commands?
    goto prompt;
  current_line +=  current_line[sizeof(LINENUM)];

execline:
    if(current_line == program_end) // Out of lines to run
    goto warmstart;
  txtpos = current_line+sizeof(LINENUM)+sizeof(char);
  goto interperateAtTxtpos;

input:
  {
    unsigned char var;
    ignore_blanks();
    if(*txtpos < 'A' || *txtpos > 'Z')
      goto qwhat;
    var = *txtpos;
    txtpos++;
    ignore_blanks();
    if(*txtpos != NL && *txtpos != ':')
      goto qwhat;
    ((short int *)variables_begin)[var-'A'] = 99;

    goto run_next_statement;
  }
forloop:
  {
    unsigned char var;
    short int initial, step, terminal;
    ignore_blanks();
    if(*txtpos < 'A' || *txtpos > 'Z')
      goto qwhat;
    var = *txtpos;
    txtpos++;
    ignore_blanks();
    if(*txtpos != '=')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    expression_error = 0;
    initial = expression();
    if(expression_error)
      goto qwhat;
  
    scantable(to_tab);
    if(table_index != 0)
      goto qwhat;
  
    terminal = expression();
    if(expression_error)
      goto qwhat;
  
    scantable(step_tab);
    if(table_index == 0)
    {
      step = expression();
      if(expression_error)
        goto qwhat;
    }
    else
      step = 1;
    ignore_blanks();
    if(*txtpos != NL && *txtpos != ':')
      goto qwhat;


    if(!expression_error && *txtpos == NL)
    {
      struct stack_for_frame *f;
      if(sp + sizeof(struct stack_for_frame) < stack_limit)
        goto qsorry;

      sp -= sizeof(struct stack_for_frame);
      f = (struct stack_for_frame *)sp;
      ((short int *)variables_begin)[var-'A'] = initial;
      f->frame_type = STACK_FOR_FLAG;
      f->for_var = var;
      f->terminal = terminal;
      f->step     = step;
      f->txtpos   = txtpos;
      f->current_line = current_line;
      goto run_next_statement;
    }
  }
  goto qhow;

gosub:
  expression_error = 0;
  linenum = expression();
  if(!expression_error && *txtpos == NL)
  {
    struct stack_gosub_frame *f;
    if(sp + sizeof(struct stack_gosub_frame) < stack_limit)
      goto qsorry;

    sp -= sizeof(struct stack_gosub_frame);
    f = (struct stack_gosub_frame *)sp;
    f->frame_type = STACK_GOSUB_FLAG;
    f->txtpos = txtpos;
    f->current_line = current_line;
    current_line = findline();
    goto execline;
  }
  goto qhow;

next:
  // Fnd the variable name
  ignore_blanks();
  if(*txtpos < 'A' || *txtpos > 'Z')
    goto qhow;
  txtpos++;
  ignore_blanks();
  if(*txtpos != ':' && *txtpos != NL)
    goto qwhat;
  
gosub_return:
  // Now walk up the stack frames and find the frame we want, if present
  tempsp = sp;
  while(tempsp < program+sizeof(program)-1)
  {
    switch(tempsp[0])
    {
      case STACK_GOSUB_FLAG:
        if(table_index == KW_RETURN)
        {
          struct stack_gosub_frame *f = (struct stack_gosub_frame *)tempsp;
          current_line  = f->current_line;
          txtpos      = f->txtpos;
          sp += sizeof(struct stack_gosub_frame);
          goto run_next_statement;
        }
        // This is not the loop you are looking for... so Walk back up the stack
        tempsp += sizeof(struct stack_gosub_frame);
        break;
      case STACK_FOR_FLAG:
        // Flag, Var, Final, Step
        if(table_index == KW_NEXT)
        {
          struct stack_for_frame *f = (struct stack_for_frame *)tempsp;
          // Is the the variable we are looking for?
          if(txtpos[-1] == f->for_var)
          {
            short int *varaddr = ((short int *)variables_begin) + txtpos[-1] - 'A'; 
            *varaddr = *varaddr + f->step;
            // Use a different test depending on the sign of the step increment
            if((f->step > 0 && *varaddr <= f->terminal) || (f->step < 0 && *varaddr >= f->terminal))
            {
              // We have to loop so don't pop the stack
              txtpos = f->txtpos;
              current_line = f->current_line;
              goto run_next_statement;
            }
            // We've run to the end of the loop. drop out of the loop, popping the stack
            sp = tempsp + sizeof(struct stack_for_frame);
            goto run_next_statement;
          }
        }
        // This is not the loop you are looking for... so Walk back up the stack
        tempsp += sizeof(struct stack_for_frame);
        break;
      default:
        //printf("Stack is stuffed!\n");
        goto warmstart;
    }
  }
  // Didn't find the variable we've been looking for
  goto qhow;

assignment:
  {
    short int value;
    short int *var;

    if(*txtpos < 'A' || *txtpos > 'Z')
      goto qhow;
    var = (short int *)variables_begin + *txtpos - 'A';
    txtpos++;

    ignore_blanks();

    if (*txtpos != '=')
      goto qwhat;
    txtpos++;
    ignore_blanks();
    expression_error = 0;
    value = expression();
    if(expression_error)
      goto qwhat;
    // Check that we are at the end of the statement
    if(*txtpos != NL && *txtpos != ':')
      goto qwhat;
    *var = value;
  }
  goto run_next_statement;

list:
  linenum = testnum(); // Retuns 0 if no line found.

  // Should be EOL
  if(txtpos[0] != NL)
    goto qwhat;

  // Find the line
  list_line = findline();
  while(list_line != program_end)
          printline();
  goto warmstart;

print:
  // If we have an empty list then just put out a NL
  if(*txtpos == ':' )
  {
        line_terminator();
    txtpos++;
    goto run_next_statement;
  }
  if(*txtpos == NL)
  {
    goto execnextline;
  }

  while(1)
  {
    ignore_blanks();
    if(print_quoted_string())
    {
      ;
    }
    else if(*txtpos == '"' || *txtpos == '\'')
      goto qwhat;
    else
    {
      short int e;
      expression_error = 0;
      e = expression();
      if(expression_error)
        goto qwhat;
      printnum(e);
    }

    // At this point we have three options, a comma or a new line
    if(*txtpos == ',')
      txtpos++; // Skip the comma and move onto the next
    else if(txtpos[0] == ';' && (txtpos[1] == NL || txtpos[1] == ':'))
    {
      txtpos++; // This has to be the end of the print - no newline
      break;
    }
    else if(*txtpos == NL || *txtpos == ':')
    {
      line_terminator();  // The end of the print statement
      break;
    }
    else
      goto qwhat; 
  }
  goto run_next_statement;

mem:
  printnum(variables_begin-program_end);
  printmsg(memorymsg);
        goto run_next_statement;
        
/*************************************************/


}

// returns 1 if the character is valid in a filename
static int isValidFnChar( char c )
{
  if( c >= '0' && c <= '9' ) return 1; // number
  if( c >= 'A' && c <= 'Z' ) return 1; // LETTER
  if( c >= 'a' && c <= 'z' ) return 1; // letter (for completeness)
  if( c == '_' ) return 1;
  if( c == '+' ) return 1;
  if( c == '.' ) return 1;
  if( c == '~' ) return 1;  // Window~1.txt
  
  return 0;
}

static unsigned char * filenameWord(void)
{
  // SDL - I wasn't sure if this functionality existed above, so I figured i'd put it here
  unsigned char * ret = txtpos;
  expression_error = 0;
  
  // make sure there are no quotes or spaces, search for valid characters
  //while(*txtpos == SPACE || *txtpos == TAB || *txtpos == SQUOTE || *txtpos == DQUOTE ) txtpos++;
  while( !isValidFnChar( *txtpos )) txtpos++;
  ret = txtpos;

  if( *ret == '\0' ) {
    expression_error = 1;
    return ret;
  }
  
  // now, find the next nonfnchar
  txtpos++;
  while( isValidFnChar( *txtpos )) txtpos++;
  if( txtpos != ret ) *txtpos = '\0';

  // set the error code if we've got no string
  if( *ret == '\0' ) {
    expression_error = 1;
  }
  
  return ret;
}

/***************************************************************************/
static void line_terminator(void)
{
    outchar(NL);
  outchar(CR);
}

/***********************************************************/
void setup()
{
/*#if ARDUINO
    Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
#endif*/
  pinMode(LED, OUTPUT);
  oled.begin();
  oled.fill(0x55);
  delay(250);
  oled.clear();
  oled.setFont(FONT6X8);
  oled.setCursor(0,0);
}

/***********************************************************/
/*static unsigned char breakcheck(void)
{
#if ARDUINO
  if(Serial.available())
    return Serial.read() == CTRLC;
  return 0;
#endif
}*/
/***********************************************************/
static int inchar()
{

/* TBC - Need to trim memory before doing this. Ideas:
 *  
 *    1. Implement ZX Basic style tokenization
 *    2. Trim more features (nah)
 *    3. Move BASIC code to EEPROM (only good for 10k writes
 *       but it does persist)
 */
 while(1){

 
  /*if (pressed == 2){
    // I really shouldn't be allowed to use a text editor after this...

    // char steves_basic_program[] = {
    // "10 PRINT \"Steve is cool.\"\n20 GOTO 10\nLIST\nRUN\n"
    // };
    //try printing just one statement
    switch(ppos){
      //case 0: ppos++; return 'M';
      //case 1: ppos++; return 'E';
      //case 2: ppos++; return 'M';
      case 3: ppos++; return CR;
      /*case 0: ppos++; return '1';
      case 1: ppos++; return '0';
      case 2: ppos++; return ' ';
      case 3: ppos++; return 'P';
      case 4: ppos++; return 'R';
      case 5: ppos++; return 'I';
      case 6: ppos++; return 'N';
      case 7: ppos++; return 'T';
      case 8: ppos++; return ' ';
      case 9: ppos++; return '"';
      case 10: ppos++; return 'S';
      case 11: ppos++; return 'T';
      case 12: ppos++; return '"';
      case 13: ppos++; line_terminator(); return 0xa; // I feel bad about this but 0xd triggers
      case 14: ppos++; return '2'; // lots of memory manipulation and we're already at the limit
      case 15: ppos++; return '0'; // so it barfs. One day I'll get this down though.
      case 16: ppos++; return ' ';
      case 17: ppos++; return 'G';
      case 18: ppos++; return 'O';
      case 19: ppos++; return 'T';
      case 20: ppos++; return 'O';
      case 21: ppos++; return ' ';
      case 22: ppos++; return '1';
      case 23: ppos++; return '0';
      case 24: ppos++; line_terminator(); ppos = 0; return 0xa;
      
    } // 5730 5754 24 bytes per char 258 left 10.75 chars left
  } 

  if (pressed != 2){
    pinMode(BUT2, INPUT_PULLUP);
    if (digitalRead(BUT2) == LOW) {
      pressed = 2;
      pinMode(BUT2, OUTPUT);
      delay(500);
    }
  }*/

 }
  
  
}

/***********************************************************/
static void outchar(unsigned char c)
{
// This is hacky af. I only did it to wind up Dominic Spill. Next stop, PHP!  
  oledChar(c);
}

static void oledChar(byte c){
  oled.print(c);
  cpos++;
  delay(20);
  if (cpos >= 112) {
  //if (cpos >= xres * (yres-1) - 1) {
    nextPage();
    cpos = 0;
    oled.setCursor(0,0);
    oled.clear();
  }
}


static void nextPage(){
  oled.setCursor(0, yres);
  oled.print(F("Next page?"));
  pinMode(BUT1, INPUT_PULLUP);
  while(1){
    if (digitalRead(BUT1) == LOW) {
        pressed = 1;
        delay(500);
    }
    if (pressed == 1){
      if (digitalRead(BUT1) != LOW){
        pinMode(BUT1, OUTPUT);
        pressed = 0;
        break;
      }
    }
  }
}




/***********************************************************/

