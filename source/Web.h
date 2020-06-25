// This file contains all operations related to the web interface
#ifndef WEB_H
#define WEB_H

#include <iostream>
#include "SymWorld.h"
#include "../../Empirical/source/web/Document.h"
#include "../../Empirical/source/web/Canvas.h"
#include "../../Empirical/source/web/web.h"

namespace UI = emp::web;

class Web {
private:
    int RECT_WIDTH = 15;
    int side_x; // how many orgs on the width
    int side_y; // how many orgs on the height
    int offset; // how much px to offset from the left side of browser

public:
    void setSideX(int x){
        side_x = x;
    }

    void setSideY(int y){
        side_y = y;
    }

    void setOffset(int off){
        offset = off;
    }

    UI::Canvas addHostCanvas(UI::Document & doc){
        return doc.AddCanvas(offset + side_x * RECT_WIDTH, offset + side_y * RECT_WIDTH, "host_can");
    }

    void drawPetriDish(UI::Canvas & can, emp::vector<emp::Ptr<Host>> & p){
        int i = 0;
        for (int x = 0; x < side_x; x++){ // now draw a virtual petri dish. 20 is the starting coordinate
            for (int y = 0; y < side_y; y++){
                std::string color;
                if (p[i]->GetIntVal() < 0) color = "blue";
                else color = "yellow";
                can.Rect(offset + x * RECT_WIDTH, offset + y * RECT_WIDTH, RECT_WIDTH, RECT_WIDTH, color, "black");
                i++;
            }
        }
    }
};

#endif