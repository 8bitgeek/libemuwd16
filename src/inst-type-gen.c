#include <stdio.h>

char instruction_type[65536];  /* maps all words to inst format type */

static void build_decode(void);

int main(int argc,char* argv[])
{
    build_decode();
    printf( "static const uint8_t instruction_type[] = {\n");
    for (int i = 0; i < 65536; i+=16)
    {
        printf ("\t/* %04X  */\t", i );
        for (int j = 0; j < 16; j++ )
        {
            int k = i+j;
            printf( "0x%02X,",instruction_type[k]);
        }
        printf ("\n", i );
    }
    printf( "};\n" );

    return 0;
}

/*-------------------------------------------------------------------*/
/* build instruction decode table                                    */
/*-------------------------------------------------------------------*/
static void build_decode(void) {
  int i, j;
  /* fill table with 0's (invalid opcode type) */
  for (i = 0; i < 65536; i++)
    instruction_type[i] = 0;
  /* fill format 1 part of table */
  for (i = 0; i < 16; i++)
    instruction_type[i] = 1;
  /* fill format 2 part of table */
  for (i = 16; i < 48; i++)
    instruction_type[i] = 2;
  /* fill format 3 part of table */
  for (i = 48; i < 64; i++)
    instruction_type[i] = 3;
  /* fill format 4 part of table */
  for (i = 64; i < 256; i++)
    instruction_type[i] = 4;
  /* fill format 5 part of table */
  for (i = 1; i < 8; i++)
    for (j = 0; j < 256; j++)
      instruction_type[j + (i * 256)] = 5;
  for (i = 0; i < 8; i++)
    for (j = 0; j < 256; j++)
      instruction_type[32768 + j + (i * 256)] = 5;
  /* fill format 6 (split ops) part of table */
  for (i = 8; i < 10; i++)
    for (j = 0; j < 256; j++)
      instruction_type[j + (i * 256)] = 6;
  for (i = 8; i < 10; i++)
    for (j = 0; j < 256; j++)
      instruction_type[32768 + j + (i * 256)] = 6;
  for (i = 14; i < 16; i++)
    for (j = 0; j < 256; j++)
      instruction_type[32768 + j + (i * 256)] = 6;
  /* fill format 7 part of table */
  for (i = 10; i < 14; i++)
    for (j = 0; j < 256; j++)
      instruction_type[j + (i * 256)] = 7;
  for (i = 10; i < 14; i++)
    for (j = 0; j < 256; j++)
      instruction_type[32768 + j + (i * 256)] = 7;
  /* fill format 8 part of table */
  for (i = 14; i < 16; i++)
    for (j = 0; j < 256; j++)
      instruction_type[j + (i * 256)] = 8;
  /* fill format 9 part of table */
  for (i = 112; i < 128; i++)
    for (j = 0; j < 256; j++)
      instruction_type[j + (i * 256)] = 9;
  /* fill format 10 part of table */
  for (i = 16; i < 112; i++)
    for (j = 0; j < 256; j++)
      instruction_type[j + (i * 256)] = 10;
  for (i = 16; i < 112; i++)
    for (j = 0; j < 256; j++)
      instruction_type[32768 + j + (i * 256)] = 10;
  /* fill format 11 part of table */
  for (i = 0; i < 5; i++)
    for (j = 0; j < 256; j++)
      instruction_type[61440 + j + (i * 256)] = 11;

} /* end function build_decode */