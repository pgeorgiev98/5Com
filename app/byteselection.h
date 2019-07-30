#ifndef SELECTION_H
#define SELECTION_H

struct ByteSelection
{
	int begin, count;
	ByteSelection(int begin, int count)
		: begin(begin), count(count) {}
};

#endif // SELECTION_H
