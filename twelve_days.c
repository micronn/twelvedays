char *a[]={
	"\n6 Geese a Lay�g�",
	" day of Christmas\nmy�rue l� s�t�o me:",
	"\n2 Turtle D�s\nand a�",
	"\n9 Ladies Danc��",
	"\n�",
	"\n4 Call� B�ds�",
	"\n10 Lor�Leap��",
	"\n7 Swan�Swimm��",
	"�artridge in a�ear Tree\n",
	"\n8 Mai�Milk��",
	"\n11�����",
	"th�",
	"\n5 Gold� R�s�",
	"\n3 Fr�ch H�s�",
	"On�he ",
	" Drumm",
	"d�",
	"�",
	"�ip",
	"ev��",
	"s a ",
	"�s",
	"�t",
	" P",
	" t",
	"en",
	"ers",
	"in",
	"�",
	"�",
	"�",
	"ing",
	"ir",
	"ove"
};
char*b="�f�st�\nA��econd���h�"
	"d���four���fif���i"
	"x������eigh���n�"
	"�������el���welf"
	"�\n12����g�";

pr(char*s)
{
	for(;*s;s++)
		if(*s&128)
			pr(a[*s&127]);
		else putchar(*s);
}
main()
{
	pr(b);
}	
