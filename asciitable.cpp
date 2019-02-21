#include "asciitable.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QFontDatabase>

AsciiTable::AsciiTable(QWidget *parent)
	: QDialog(parent)
{
	QHBoxLayout *layout = new QHBoxLayout;
	setLayout(layout);

	QLabel *label = new QLabel;
	layout->addWidget(label);

	label->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	label->setTextInteractionFlags(Qt::TextSelectableByMouse);

	QString text = "\
┌──────────────────────────────────────────────┬────────────────────────────┬────────────────────────┐\n\
│ Oct   Dec   Hex   Char                       │ Oct   Dec   Hex   Char     │ Oct   Dec   Hex   Char │\n\
├──────────────────────────────────────────────┼────────────────────────────┼────────────────────────┤\n\
│ 000   0     00    NUL (null character)       │ 053   43    2B    +        │ 126   86    56    V    │\n\
│ 001   1     01    SOH (start of heading)     │ 054   44    2C    ,        │ 127   87    57    W    │\n\
│ 002   2     02    STX (start of text)        │ 055   45    2D    -        │ 130   88    58    X    │\n\
│ 003   3     03    ETX (end of text)          │ 056   46    2E    .        │ 131   89    59    Y    │\n\
│ 004   4     04    EOT (end of transmission)  │ 057   47    2F    /        │ 132   90    5A    Z    │\n\
│ 005   5     05    ENQ (enquiry)              │ 060   48    30    0        │ 133   91    5B    [    │\n\
│ 006   6     06    ACK (acknowledge)          │ 061   49    31    1        │ 134   92    5C    \\    │\n\
│ 007   7     07    BEL (bell)                 │ 062   50    32    2        │ 135   93    5D    ]    │\n\
│ 010   8     08    BS  (backspace)            │ 063   51    33    3        │ 136   94    5E    ^    │\n\
│ 011   9     09    HT  (horizontal tab)       │ 064   52    34    4        │ 137   95    5F    _    │\n\
│ 012   10    0A    LF  (new line)             │ 065   53    35    5        │ 140   96    60    `    │\n\
│ 013   11    0B    VT  (vertical tab)         │ 066   54    36    6        │ 141   97    61    a    │\n\
│ 014   12    0C    FF  (form feed)            │ 067   55    37    7        │ 142   98    62    b    │\n\
│ 015   13    0D    CR  (carriage ret)         │ 070   56    38    8        │ 143   99    63    c    │\n\
│ 016   14    0E    SO  (shift out)            │ 071   57    39    9        │ 144   100   64    d    │\n\
│ 017   15    0F    SI  (shift in)             │ 072   58    3A    :        │ 145   101   65    e    │\n\
│ 020   16    10    DLE (data link escape)     │ 073   59    3B    ;        │ 146   102   66    f    │\n\
│ 021   17    11    DC1 (device control 1)     │ 074   60    3C    <        │ 147   103   67    g    │\n\
│ 022   18    12    DC2 (device control 2)     │ 075   61    3D    =        │ 150   104   68    h    │\n\
│ 023   19    13    DC3 (device control 3)     │ 076   62    3E    >        │ 151   105   69    i    │\n\
│ 024   20    14    DC4 (device control 4)     │ 077   63    3F    ?        │ 152   106   6A    j    │\n\
│ 025   21    15    NAK (negative ack.)        │ 100   64    40    @        │ 153   107   6B    k    │\n\
│ 026   22    16    SYN (synchronous idle)     │ 101   65    41    A        │ 154   108   6C    l    │\n\
│ 027   23    17    ETB (end of trans. blk)    │ 102   66    42    B        │ 155   109   6D    m    │\n\
│ 030   24    18    CAN (cancel)               │ 103   67    43    C        │ 156   110   6E    n    │\n\
│ 031   25    19    EM  (end of medium)        │ 104   68    44    D        │ 157   111   6F    o    │\n\
│ 032   26    1A    SUB (substitute)           │ 105   69    45    E        │ 160   112   70    p    │\n\
│ 033   27    1B    ESC (escape)               │ 106   70    46    F        │ 161   113   71    q    │\n\
│ 034   28    1C    FS  (file separator)       │ 107   71    47    G        │ 162   114   72    r    │\n\
│ 035   29    1D    GS  (group separator)      │ 110   72    48    H        │ 163   115   73    s    │\n\
│ 036   30    1E    RS  (record separator)     │ 111   73    49    I        │ 164   116   74    t    │\n\
│ 037   31    1F    US  (unit separator)       │ 112   74    4A    J        │ 165   117   75    u    │\n\
│ 040   32    20    SPACE                      │ 113   75    4B    K        │ 166   118   76    v    │\n\
│ 041   33    21    !                          │ 114   76    4C    L        │ 167   119   77    w    │\n\
│ 042   34    22    \"                          │ 115   77    4D    M        │ 170   120   78    x    │\n\
│ 043   35    23    #                          │ 116   78    4E    N        │ 171   121   79    y    │\n\
│ 044   36    24    $                          │ 117   79    4F    O        │ 172   122   7A    z    │\n\
│ 045   37    25    %                          │ 120   80    50    P        │ 173   123   7B    {    │\n\
│ 046   38    26    &                          │ 121   81    51    Q        │ 174   124   7C    |    │\n\
│ 047   39    27    '                          │ 122   82    52    R        │ 175   125   7D    }    │\n\
│ 050   40    28    (                          │ 123   83    53    S        │ 176   126   7E    ~    │\n\
│ 051   41    29    )                          │ 124   84    54    T        │ 177   127   7F    DEL  │\n\
│ 052   42    2A    *                          │ 125   85    55    U        │                        │\n\
└──────────────────────────────────────────────┴────────────────────────────┴────────────────────────┘";

	label->setText(text);
}
