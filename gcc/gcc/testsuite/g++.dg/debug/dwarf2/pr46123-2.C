// PR debug/46123
// { dg-do compile }
// { dg-options "-g -feliminate-dwarf2-dups" }

struct foo
{
  static int bar ()
  {
    int i;
    static int baz = 1;
    {
      static int baz = 2;
      i = baz++;
    }
    {
      struct baz
      {
	static int m ()
	{
	  static int n;
	  return n += 10;
	}
      };
      baz a;
      i += a.m ();
    }
    {
      static int baz = 3;
      i += baz;
      baz += 30;
    }
    i += baz;
    baz += 60;
    return i;
  }
};

int main ()
{
  foo x;

  if (x.bar () != 16)
    return 1;
  if (x.bar() != 117)
    return 1;
  return 0;
}

/* { dg-bogus "-feliminate-dwarf2-dups is broken for C\\+\\+, ignoring" "broken -feliminate-dwarf2-dups" { xfail *-*-* } 1 } */
