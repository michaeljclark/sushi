
#line 1 "sushi/visual_studio_parser.rl"
//
//  project_parser.rl
//

#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cctype>
#include <cstring>

#include "visual_studio_parser.h"


#line 44 "sushi/visual_studio_parser.rl"



#line 21 "sushi/visual_studio_parser.cc"
static const char _visual_studio_parser_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1, 
	3, 1, 4, 1, 5
};

static const unsigned char _visual_studio_parser_key_offsets[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 
	7, 8, 9, 10, 11, 12, 13, 14, 
	15, 16, 17, 18, 19, 20, 21, 22, 
	23, 24, 25, 26, 27, 28, 29, 30, 
	31, 32, 33, 34, 35, 36, 37, 38, 
	39, 40, 41, 42, 43, 44, 45, 46, 
	47, 48, 49, 50, 51, 52, 53, 54, 
	55, 56, 57, 58, 59, 62, 66, 67, 
	68, 69, 70, 71, 72, 73, 74, 75, 
	76, 77, 78, 79, 80, 81, 82, 83, 
	86, 90, 91, 92, 93, 94, 95, 96, 
	97, 98, 99, 100, 101, 102, 103, 104, 
	105, 106, 107, 108, 109, 110, 111, 112, 
	113, 114, 115, 116, 117, 118, 119, 122, 
	126, 127, 128, 129, 130, 131, 132, 133, 
	134, 135, 136, 137, 138, 139, 140, 141, 
	142, 143, 144, 145, 146, 147, 148, 151, 
	155, 156, 158, 158
};

static const char _visual_studio_parser_trans_keys[] = {
	-17, -69, -65, 13, 10, 77, 105, 99, 
	114, 111, 115, 111, 102, 116, 32, 86, 
	105, 115, 117, 97, 108, 32, 83, 116, 
	117, 100, 105, 111, 32, 83, 111, 108, 
	117, 116, 105, 111, 110, 32, 70, 105, 
	108, 101, 44, 32, 70, 111, 114, 109, 
	97, 116, 32, 86, 101, 114, 115, 105, 
	111, 110, 32, 46, 48, 57, 13, 46, 
	48, 57, 10, 35, 32, 86, 105, 115, 
	117, 97, 108, 32, 83, 116, 117, 100, 
	105, 111, 32, 46, 48, 57, 13, 46, 
	48, 57, 10, 105, 110, 105, 109, 117, 
	109, 86, 105, 115, 117, 97, 108, 83, 
	116, 117, 100, 105, 111, 86, 101, 114, 
	115, 105, 111, 110, 32, 61, 32, 46, 
	48, 57, 13, 46, 48, 57, 10, 105, 
	115, 117, 97, 108, 83, 116, 117, 100, 
	105, 111, 86, 101, 114, 115, 105, 111, 
	110, 32, 61, 32, 46, 48, 57, 13, 
	46, 48, 57, 10, 77, 86, 77, 0
};

static const char _visual_studio_parser_single_lengths[] = {
	0, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 2, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	2, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 2, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 2, 
	1, 2, 0, 1
};

static const char _visual_studio_parser_range_lengths[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 1, 1, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 1, 
	1, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 1, 1, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 1, 1, 
	0, 0, 0, 0
};

static const short _visual_studio_parser_index_offsets[] = {
	0, 0, 2, 4, 6, 8, 10, 12, 
	14, 16, 18, 20, 22, 24, 26, 28, 
	30, 32, 34, 36, 38, 40, 42, 44, 
	46, 48, 50, 52, 54, 56, 58, 60, 
	62, 64, 66, 68, 70, 72, 74, 76, 
	78, 80, 82, 84, 86, 88, 90, 92, 
	94, 96, 98, 100, 102, 104, 106, 108, 
	110, 112, 114, 116, 118, 121, 125, 127, 
	129, 131, 133, 135, 137, 139, 141, 143, 
	145, 147, 149, 151, 153, 155, 157, 159, 
	162, 166, 168, 170, 172, 174, 176, 178, 
	180, 182, 184, 186, 188, 190, 192, 194, 
	196, 198, 200, 202, 204, 206, 208, 210, 
	212, 214, 216, 218, 220, 222, 224, 227, 
	231, 233, 235, 237, 239, 241, 243, 245, 
	247, 249, 251, 253, 255, 257, 259, 261, 
	263, 265, 267, 269, 271, 273, 275, 278, 
	282, 284, 287, 288
};

static const unsigned char _visual_studio_parser_indicies[] = {
	0, 1, 2, 1, 3, 1, 4, 1, 
	5, 1, 6, 1, 7, 1, 8, 1, 
	9, 1, 10, 1, 11, 1, 12, 1, 
	13, 1, 14, 1, 15, 1, 16, 1, 
	17, 1, 18, 1, 19, 1, 20, 1, 
	21, 1, 22, 1, 23, 1, 24, 1, 
	25, 1, 26, 1, 27, 1, 28, 1, 
	29, 1, 30, 1, 31, 1, 32, 1, 
	33, 1, 34, 1, 35, 1, 36, 1, 
	37, 1, 38, 1, 39, 1, 40, 1, 
	41, 1, 42, 1, 43, 1, 44, 1, 
	45, 1, 46, 1, 47, 1, 48, 1, 
	49, 1, 50, 1, 51, 1, 52, 1, 
	53, 1, 54, 1, 55, 1, 56, 1, 
	57, 1, 58, 1, 59, 1, 60, 60, 
	1, 61, 62, 62, 1, 63, 1, 64, 
	1, 65, 1, 66, 1, 67, 1, 68, 
	1, 69, 1, 70, 1, 71, 1, 72, 
	1, 73, 1, 74, 1, 75, 1, 76, 
	1, 77, 1, 78, 1, 79, 1, 80, 
	80, 1, 81, 82, 82, 1, 83, 1, 
	84, 1, 85, 1, 86, 1, 87, 1, 
	88, 1, 89, 1, 90, 1, 91, 1, 
	92, 1, 93, 1, 94, 1, 95, 1, 
	96, 1, 97, 1, 98, 1, 99, 1, 
	100, 1, 101, 1, 102, 1, 103, 1, 
	104, 1, 105, 1, 106, 1, 107, 1, 
	108, 1, 109, 1, 110, 1, 111, 1, 
	112, 112, 1, 113, 114, 114, 1, 115, 
	1, 116, 1, 117, 1, 118, 1, 119, 
	1, 120, 1, 121, 1, 122, 1, 123, 
	1, 124, 1, 125, 1, 126, 1, 127, 
	1, 128, 1, 129, 1, 130, 1, 131, 
	1, 132, 1, 133, 1, 134, 1, 135, 
	1, 136, 1, 137, 137, 1, 138, 139, 
	139, 1, 140, 1, 141, 142, 1, 1, 
	141, 1, 0
};

static const unsigned char _visual_studio_parser_trans_targs[] = {
	2, 0, 3, 4, 5, 6, 7, 8, 
	9, 10, 11, 12, 13, 14, 15, 16, 
	17, 18, 19, 20, 21, 22, 23, 24, 
	25, 26, 27, 28, 29, 30, 31, 32, 
	33, 34, 35, 36, 37, 38, 39, 40, 
	41, 42, 43, 44, 45, 46, 47, 48, 
	49, 50, 51, 52, 53, 54, 55, 56, 
	57, 58, 59, 60, 61, 62, 61, 63, 
	64, 65, 66, 67, 68, 69, 70, 71, 
	72, 73, 74, 75, 76, 77, 78, 79, 
	80, 81, 80, 137, 83, 84, 85, 86, 
	87, 88, 89, 90, 91, 92, 93, 94, 
	95, 96, 97, 98, 99, 100, 101, 102, 
	103, 104, 105, 106, 107, 108, 109, 110, 
	111, 112, 111, 138, 114, 115, 116, 117, 
	118, 119, 120, 121, 122, 123, 124, 125, 
	126, 127, 128, 129, 130, 131, 132, 133, 
	134, 135, 136, 135, 139, 82, 113
};

static const char _visual_studio_parser_trans_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 1, 3, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	1, 5, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	1, 9, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 1, 7, 0, 0, 0, 0
};

static const char _visual_studio_parser_eof_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 11, 11, 11
};

static const int visual_studio_parser_start = 1;
static const int visual_studio_parser_first_final = 137;
static const int visual_studio_parser_error = 0;

static const int visual_studio_parser_en_main = 1;


#line 47 "sushi/visual_studio_parser.rl"

bool VisualStudioParser::parse(const char *buffer, size_t len)
{
    int cs = visual_studio_parser_en_main;
    
    const char *mark = NULL;
    const char *p = buffer;
    const char *pe = buffer + strlen(buffer);
    const char *eof = pe;

    
#line 256 "sushi/visual_studio_parser.cc"
	{
	cs = visual_studio_parser_start;
	}

#line 58 "sushi/visual_studio_parser.rl"
    
#line 263 "sushi/visual_studio_parser.cc"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _visual_studio_parser_trans_keys + _visual_studio_parser_key_offsets[cs];
	_trans = _visual_studio_parser_index_offsets[cs];

	_klen = _visual_studio_parser_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _visual_studio_parser_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _visual_studio_parser_indicies[_trans];
	cs = _visual_studio_parser_trans_targs[_trans];

	if ( _visual_studio_parser_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _visual_studio_parser_actions + _visual_studio_parser_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 17 "sushi/visual_studio_parser.rl"
	{ mark = p; }
	break;
	case 1:
#line 18 "sushi/visual_studio_parser.rl"
	{ FormatVersion(mark, p - mark); }
	break;
	case 2:
#line 19 "sushi/visual_studio_parser.rl"
	{ CommentVersion(mark, p - mark); }
	break;
	case 3:
#line 20 "sushi/visual_studio_parser.rl"
	{ VisualStudioVersion(mark, p - mark); }
	break;
	case 4:
#line 21 "sushi/visual_studio_parser.rl"
	{ MinimumVisualStudioVersion(mark, p - mark); }
	break;
#line 357 "sushi/visual_studio_parser.cc"
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	const char *__acts = _visual_studio_parser_actions + _visual_studio_parser_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 5:
#line 23 "sushi/visual_studio_parser.rl"
	{ 
        Done();
        {p++; goto _out; }
    }
	break;
#line 380 "sushi/visual_studio_parser.cc"
		}
	}
	}

	_out: {}
	}

#line 59 "sushi/visual_studio_parser.rl"

    return (cs != visual_studio_parser_error || cs == visual_studio_parser_first_final);
}
