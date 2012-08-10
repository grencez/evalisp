
#include "cx/syscx.h"
#include "cx/cons.h"
#include "cx/fileb.h"

#include <string.h>

    Cons*
parse_lisp_XFileB (XFileB* xf)
{
    char delims[2+sizeof(WhiteSpaceChars)];
    char* s = 0;
    char c = 0;
    DeclTable( Cons, up );
    Cons* x = Grow1Table( up );
    x->cdr = 0;

    delims[0] = '(';
    delims[1] = ')';
    strcpy (&delims[2], WhiteSpaceChars);

    for (s = nextds_XFileB (xf, &c, delims);
         s;
         s = nextds_XFileB (xf, &c, delims))
    {
        Cons* y;
        if (s[0] != '\0')
        {
            x->cdr = make_Cons ();
            x = x->cdr;
            x->car.kind = Cons_cstr;
            x->car.as.cstr = dup_cstr (s);
        }

        if (c == '(')
        {
            x->cdr = make_Cons ();
            y = x->cdr;
            x = Grow1Table( up );
            x->cdr = 0;
            x->car.as.cons = y;
        }
        else if (c == ')')
        {
            if (up.sz == 1)
            {
                DBog0( "Too many closed parens!" );
                break;
            }

            y = &up.s[up.sz-1];
            x = y->car.as.cons;
            x->car.kind = Cons_Cons;
            x->car.as.cons = y->cdr;
            MPopTable( up, 1 );
        }
    }

    if (up.sz > 1)
    {
        { BLoop( i, up.sz-1 )
            Cons* y = &up.s[i+1];
            if (y->cdr)
            {
                lose_Cons (y->cdr);
                y->cdr = 0;
            }
        } BLose()
        DBog1( "%u paren pairs need closing!", up.sz-1 );
    }

    x = up.s[0].cdr;
    LoseTable( up );
    return x;
}


    int
main (int argc, char** argv)
{
    int argi =
        (init_sysCx (&argc, &argv),
         1);
    XFileB* xf = stdin_XFileB ();
    OFileB* of = stderr_OFileB ();
    Cons* x;

    if (argi < argc)
        failout_sysCx ("I don't take arguments from humans.");

    x = parse_lisp_XFileB (xf);
    dump_Cons (of, x);
    lose_Cons (x);

    dump_char_OFileB (of, '\n');

    lose_sysCx ();
    return 0;
}

