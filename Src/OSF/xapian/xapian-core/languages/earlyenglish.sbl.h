/* Generated by Snowball 2.0.0 - https://snowballstem.org/ */


#include "steminternal.h"

namespace Xapian {

class InternalStemEarlyenglish : public SnowballStemImplementation {
    uchar I_Y_found;
    int I_p2;
    int I_p1;
    int r_Step_5b();
    int r_Step_5a();
    int r_Step_4();
    int r_Step_3();
    int r_Step_2();
    int r_Step_1c();
    int r_Step_1b();
    int r_Step_1a();
    int r_R2();
    int r_R1();
    int r_shortv();

  public:

	InternalStemEarlyenglish();
	~InternalStemEarlyenglish();
	int stem();
	std::string get_description() const;
};

}
