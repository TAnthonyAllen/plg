class DoubleLinkList;
class PLGset;
class Buffer;
class PLGparse;
class PLGitem;
/*******************************************************************************
	Class that embodies the Alternative rule in plg
*******************************************************************************/

class Alternative
{
public:
DoubleLinkList *elements;
PLGset *guardSet;
Alternative();
void generate(Buffer *output);
int match(PLGparse *state, PLGitem *&out);
};
