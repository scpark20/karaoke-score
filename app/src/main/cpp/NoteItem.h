//
// Created by Park, Soo-Chul on 10/10/2017.
//

#ifndef KARAOKESCORE_NOTEITEM_H
#define KARAOKESCORE_NOTEITEM_H


class NoteItem {
public:
    NoteItem(double start, double end, double note):
    start(start), end(end), note(note) {

    }

    double start = 0;
    double end = 0;
    double note = 0;

private:

};


#endif //KARAOKESCORE_NOTEITEM_H
