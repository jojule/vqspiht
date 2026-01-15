#if ! defined(__QM_H)
#define __QM_H

/*--------------------------- Counters -------------------------------*/

extern long   BytesInQM;      /* Number of bytes read  at decoding stage */
extern long   BytesOutQM;     /* Number of bytes write at encoding stage */
extern int    EscMode;        /* Mode with escape codes                  */
extern long   EndCodePos;     /* instead End-Code-sumbol                 */

/*---------------- Interface for standard QM-coder -------------------*/

int    DecodeBitByQM( FILE* f, int state);
void   EncodeBitByQM( FILE* f, int state, int bit);
void   InitDecodeQM(FILE* f);
void   InitEncodeQM(void);
void   NewModel(int MaxStates);
void   InitModelQM( int NumberOfStates );
void   FlushEncodeQM( FILE* f);
float  GetProbabilityQM(int bit, int state);
void   DoneQM(void);

/*----------------- Interface for semi-adative QM --------------------*/

int    GetStateIndex(float whiteprob);
void   RestoreStateIndex(int state, int value);

#endif /* __QM_H */
