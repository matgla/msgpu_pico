#include <algorithm>
#include <span>
#include <iostream>
#include <cstdio>
#include <array>
#include <cmath>

struct vertex
{
    uint16_t y;
    uint16_t x;
};

struct triangle
{
    std::array<vertex, 3> v;
    char font;
};

struct prepared_triangle
{
    float dx1;
    float dx2;
    float dx3;
    float sx;
    float ex;
    uint16_t mid_y;
    uint16_t min_y;
    uint16_t max_y;
    char font;
};

char buffer[15][15];

void clear_buffer()
{
    for (auto& line : buffer)
    {
        for (auto& byte : line)
        {
            byte = '.';
        }
    }
}

std::vector<triangle> triangles;

void print_triangles()
{
    for (auto& triangle : triangles)
    {
        std::cout << "Triangle: {";
        for (auto& v : triangle.v)
        {
            std::cout << "{x: " << v.x << ", y: " << v.y << "}, ";
        }
        std::cout << "}" << std::endl;
    }
}

void draw_line(uint16_t y, uint16_t x1, uint16_t x2, char font)
{
    if (x1 > x2)
    {
        std::swap(x1, x2);
    }
    for (std::size_t i = x1; i <= x2; ++i)
    {
        buffer[y][i] = font;
    }
}

prepared_triangle prepare_triangle(vertex A, vertex B, vertex C, char font)
{
    float dx1, dx2, dx3;
    const float dyba = B.y - A.y;
    const float dyca = C.y - A.y;
    const float dycb = C.y - B.y;
    const float dxba = B.x - A.x;
    const float dxca = C.x - A.x;
    const float dxcb = C.x - B.x;

    if (std::abs(dyba) > 0) dx1=dxba/dyba; else dx1=dxba;
	if (std::abs(dyca) > 0) dx2=dxca/dyca; else dx2=dxca;
	if (std::abs(dycb) > 0) dx3=dxcb/dycb; else dx3=dxcb;

    std::cout << "Calculated dx1: " << dx1 << ", dx2: " << dx2 << ", dx3: " << dx3 << std::endl;
    float sx = A.x + 0.0001f;
    float ex = (A.y < B.y ? A.x : B.x) - 0.0001f;

    if (C.y < B.y)
    {
        std::swap(dx1, dx2);
    }
    return prepared_triangle {
        .dx1 = dx1,
        .dx2 = dx2,
        .dx3 = dx3,
        .sx = sx,
        .ex = ex,
        .mid_y = std::min(B.y, C.y),
        .min_y = A.y,
        .max_y = std::max(B.y, C.y),
        .font = font
    };
}

void draw_triangle(int line, prepared_triangle& t)
{
    if (line < t.min_y || line > t.max_y)
    {
        return;
    }

    const float e_dx = line < t.mid_y ? t.dx1 : t.dx3; 

    draw_line(line, round(t.sx), round(t.ex), t.font);

    t.sx += t.dx2;
    t.ex += e_dx;
}

void draw_triangle_lines(int line, prepared_triangle& t)
{
    if (line < t.min_y || line > t.max_y)
    {
        return;
    }

    float s_dx = t.dx2;
    float e_dx = line < t.mid_y ? t.dx1 : t.dx3;
    
    float prev_sx = t.sx;
    float prev_ex = t.ex;


    if (line != t.max_y && s_dx > 1)
    {
        prev_sx += s_dx - 1.0f;
    }

    if (line != t.max_y && e_dx > 1)
    {
        prev_ex += e_dx - 1.0f;
    }

    if (line != t.max_y && s_dx < -1)
    {
        prev_sx += s_dx + 1.0f;
    }

    if (line != t.max_y && e_dx < -1)
    {
        prev_ex += e_dx + 1.0f;
    }

 
    if ((t.mid_y == t.max_y || t.mid_y == t.min_y)  && t.mid_y == line)
    {
        draw_line(line, round(t.sx), round(t.ex), 'x');
    }
    
    draw_line(line, round(t.sx), round(prev_sx), 'x');
    draw_line(line, round(t.ex), round(prev_ex), 'x');

    t.sx += s_dx;
    t.ex += e_dx;
}

void render(prepared_triangle t)
{
    for (int line = 0; line < 15; ++line)
    {
        //draw_triangle(line, t);
        draw_triangle_lines(line, t);
    }
}

void print()
{
    std::cout << "  ";
    for (int i = 0; i < 15; ++i)
    {
        std::cout << (i % 10) << " ";
    }
    std::cout << std::endl;
    int i = 0;
    for (auto& line : buffer)
    {
        std::cout << (i++ % 10) << " ";
        for (auto& byte : line)
        {
            std::cout << byte << " ";
        }
        std::cout << std::endl;
    }
}

void render_triangle(std::vector<triangle> triangles)
{
    clear_buffer();
    for (auto t : triangles)
    {
    printf("Render 1 - A{%d, %d}, B{%d, %d}, C{%d, %d}\n", 
        t.v[0].y, t.v[0].x,
        t.v[1].y, t.v[1].x,
        t.v[2].y, t.v[2].x);

    render(prepare_triangle(t.v[0], t.v[1], t.v[2], t.font));
    } 
    std::cout << std::endl;
    print();
    std::cout << std::endl;
}

int main() 
{
    std::cout << "sizeof(prepared_triangle): " << sizeof(prepared_triangle)  << std::endl;
    clear_buffer();
  
    triangle t = {
        .v { vertex{1, 5}, 
            vertex{9, 9}, 
            vertex{9, 1} 
        },
        .font = 'X' 
    };

    render_triangle({t});
    
    t = {
        .v { vertex{1, 2}, 
            vertex{1, 12}, 
            vertex{13, 7} 
        },
        .font = 'X' 
    };

    render_triangle({t});
    
    t = {
        .v { vertex{0, 0}, 
            vertex{8, 14}, 
            vertex{8, 0} 
        },
        .font = 'X' 
    };

    render_triangle({t});
    
    t = {
        .v { vertex{0, 8}, 
            vertex{8, 8}, 
            vertex{8, 0} 
        },
        .font = 'X' 
    };

    render_triangle({t});
    t = {
        .v { vertex{0, 6}, 
            vertex{8, 10}, 
            vertex{12, 2} 
        },
        .font = 'X' 
    };

    render_triangle({t});
  
    t = {
        .v { vertex{0, 6}, 
            vertex{12, 10}, 
            vertex{6, 2} 
        },
        .font = 'X' 
    };
    render_triangle({t});

    t = {
        . v { vertex{7, 5},
            vertex{7, 10},
            vertex{10, 5}
        },
        .font = 'O'
    };
    render_triangle({t});

    t = {
        .v { vertex{0, 2}, 
            vertex{0, 8}, 
            vertex{8, 0} 
        },
        .font = 'X' 
    };

        triangle t2 = {
        .v { vertex{0, 8}, 
            vertex{8, 6}, 
            vertex{8, 0} 
        },
        .font = 'X' 
    };
    render_triangle({t, t2});
 
}
