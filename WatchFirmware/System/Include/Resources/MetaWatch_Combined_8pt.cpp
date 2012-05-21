// Hand-made file to make a nice lower caps font using
// the provided small and large 8 point fonts
#include "MetaWatch_Large_8pt.h"
#include "MetaWatch_Small_8pt.h"

CompositeFont MetaWatch_Combined_8pt_funky(&MetaWatch_Large_8pt, &MetaWatch_Small_8pt, false);
CompositeFont MetaWatch_Combined_8pt(&MetaWatch_Large_8pt, &MetaWatch_Small_8pt, true);
