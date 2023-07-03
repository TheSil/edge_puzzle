import svgwrite

DIM = 120

class SquarePattern:
    def __init__(self, color1, color2):
        self.color1 = color1
        self.color2 = color2

    def save(self, filename):
        dwg = svgwrite.Drawing(filename, size=('120', '120'))
        dwg.add(dwg.rect((0, 0), (DIM, DIM), fill=self.color1))
        dwg.add(dwg.rect((20, 20), (80, 80), fill=self.color2))
        dwg.add(dwg.rect((40, 40), (40, 40), fill=self.color1))
        dwg.save()

class MultiSquarePattern:
    def __init__(self, color1, color2):
        self.color1 = color1
        self.color2 = color2

    def save(self, filename):
        dim = 120
        inner_rect_dim = 55
        border_rect_dim = 25
        border_rect_offset = 18

        dwg = svgwrite.Drawing(filename, size=(str(dim), str(dim)))
        dwg.add(dwg.rect((0, 0), (dim, dim), fill=self.color1))
        dwg.add(dwg.rect(((dim - inner_rect_dim)//2, (dim - inner_rect_dim)//2),
                         (inner_rect_dim, inner_rect_dim),
                         fill=self.color2))
        dwg.add(dwg.rect((border_rect_offset, border_rect_offset),
                         (border_rect_dim, border_rect_dim), fill=self.color2))
        dwg.add(dwg.rect((dim - border_rect_dim - border_rect_offset, border_rect_offset),
                         (border_rect_dim, border_rect_dim), fill=self.color2))
        dwg.add(dwg.rect((border_rect_offset, dim - border_rect_dim - border_rect_offset),
                         (border_rect_dim, border_rect_dim), fill=self.color2))
        dwg.add(dwg.rect((dim - border_rect_dim - border_rect_offset, dim - border_rect_dim - border_rect_offset),
                         (border_rect_dim, border_rect_dim), fill=self.color2))
        dwg.save()

class PlusPattern:
    def __init__(self, color1, color2):
        self.color1 = color1
        self.color2 = color2

    def save(self, filename):
        dim = 120
        dwg = svgwrite.Drawing(filename, size=(str(dim), str(dim)))

        R = 46
        rect_width = 58
        rect_height = 18

        dwg.add(dwg.rect((0, 0), (dim, dim),
                         fill=self.color1))
        dwg.add(dwg.circle((dim//2, dim//2), R,
                           fill=self.color2))
        dwg.add(dwg.rect(((dim - rect_height)//2, (dim - rect_width)//2),
                          (rect_height, rect_width),
                         fill=self.color1))
        dwg.add(dwg.rect(((dim - rect_width)//2, (dim - rect_height)//2),
                          (rect_width, rect_height),
                         fill=self.color1))
        dwg.save()

class RoundCrossPattern:
    def __init__(self, color1, color2):
        self.color1 = color1
        self.color2 = color2

    def save(self, filename):
        dim = 120
        dwg = svgwrite.Drawing(filename, size=(str(dim), str(dim)))
        dwg.add(dwg.rect((0, 0), (dim, dim), fill=self.color1))

        R = 17
        R_offset = 35
        rect_width = 18

        dwg.add(dwg.circle((R_offset, R_offset), R, fill=self.color2))
        dwg.add(dwg.circle((dim - R_offset, R_offset), R, fill=self.color2))
        dwg.add(dwg.circle((R_offset, dim - R_offset), R, fill=self.color2))
        dwg.add(dwg.circle((dim - R_offset, dim - R_offset), R, fill=self.color2))
        dwg.add(dwg.rect((R_offset-R, dim//2 - rect_width//2),
                         (dim-2*(R_offset-R), rect_width),
                         fill=self.color2,
                         transform="rotate(45,60,60)"))
        dwg.add(dwg.rect((R_offset-R, dim//2 - rect_width//2),
                         (dim-2*(R_offset-R), rect_width),
                         fill=self.color2,
                         transform="rotate(-45,60,60)"))
        dwg.save()

class CrossPattern:
    def __init__(self, color1, color2):
        self.color1 = color1
        self.color2 = color2

    def save(self, filename):
        dim = 120
        dwg = svgwrite.Drawing(filename, size=(str(dim), str(dim)))
        dwg.add(dwg.rect((0, 0), (dim, dim), fill=self.color1))

        border_offset = 20
        offset1 = 14
        offset2 = 13
        points = ((border_offset,border_offset),
                  (dim//2-offset1,border_offset),
                  (dim//2, dim//2-offset2),
                  (dim//2, dim//2),
                  (dim//2-offset2, dim//2),
                  (border_offset,dim//2-offset1))

        dwg.add(dwg.polygon(points, fill=self.color2))
        dwg.add(dwg.polygon(points, fill=self.color2, transform="rotate(90,60,60)"))
        dwg.add(dwg.polygon(points, fill=self.color2, transform="rotate(180,60,60)"))
        dwg.add(dwg.polygon(points, fill=self.color2, transform="rotate(270,60,60)"))
        dwg.save()

class StarPattern:
    def __init__(self, color1, color2):
        self.color1 = color1
        self.color2 = color2

    def save(self, filename):
        dwg = svgwrite.Drawing(filename, size=('120', '120'))
        dwg.add(dwg.rect((0, 0), (120, 120), fill=self.color1))

        R = 45
        r = 22
        sqrt3_2 = 0.8660254040

        outer_points = ((0,R),(sqrt3_2*R,0.5*R),(sqrt3_2*R,-0.5*R),(0,-R),(-sqrt3_2*R,-0.5*R),(-sqrt3_2*R,0.5*R))
        inner_points = ((0.5*r,sqrt3_2*r),(r,0),(0.5*r,-sqrt3_2*r),(-0.5*r,-sqrt3_2*r),(-r,0),(-0.5*r,sqrt3_2*r))

        points = []
        for i in range(len(outer_points)):
            points.append(outer_points[i])
            points.append(inner_points[i])
        dwg.add(dwg.polygon(points, fill=self.color2, transform="translate(60,60) rotate(-45,0,0)"))
        dwg.save()

class CircleSubtractPattern:
    def __init__(self, color1, color2):
        self.color1 = color1
        self.color2 = color2

    def save(self, filename):
        dwg = svgwrite.Drawing(filename, size=('120', '120'))
        clip_path = dwg.defs.add(dwg.clipPath(id='clipsq'))
        clip_path.add(dwg.rect((0, 0), (120, 120)))
        defs_g = dwg.defs.add(dwg.g(clip_path='url(#clipsq)'))

        defs_g.add(dwg.rect((0, 0), (120, 120), fill=self.color1))
        defs_g.add(dwg.circle((60, 60), 50, fill=self.color2))
        defs_g.add(dwg.circle((60, 10), 22, fill=self.color1))
        defs_g.add(dwg.circle((10, 60), 22, fill=self.color1))
        defs_g.add(dwg.circle((110, 60), 22, fill=self.color1))
        defs_g.add(dwg.circle((60, 110), 22, fill=self.color1))
        dwg.add(dwg.use(defs_g, insert=(0, 0)))
        dwg.save()

class FlowerPattern:
    def __init__(self, color1, color2):
        self.color1 = color1
        self.color2 = color2

    def save(self, filename):
        dim = 120
        dwg = svgwrite.Drawing(filename, size=(str(dim), str(dim)))

        r_offset = 42
        r = 21
        R = 21

        dwg.add(dwg.rect((0, 0), (dim, dim), fill=self.color1))
        dwg.add(dwg.circle((r_offset, r_offset), r, fill=self.color2))
        dwg.add(dwg.circle((dim-r_offset, r_offset), r, fill=self.color2))
        dwg.add(dwg.circle((r_offset, dim-r_offset), r, fill=self.color2))
        dwg.add(dwg.circle((dim-r_offset, dim-r_offset), r, fill=self.color2))
        dwg.add(dwg.circle((60, 60), R, fill=self.color1))
        dwg.save()

class CircleSubtractPattern2:
    def __init__(self, color1, color2):
        self.color1 = color1
        self.color2 = color2

    def save(self, filename):
        dim = 120
        dwg = svgwrite.Drawing(filename, size=(str(dim), str(dim)))
        clip_path = dwg.defs.add(dwg.clipPath(id='clipsq'))
        clip_path.add(dwg.rect((0, 0), (dim, dim)))
        defs_g = dwg.defs.add(dwg.g(clip_path='url(#clipsq)'))

        R = 32
        R_offset = 8
        rotated_rect_dim = 105
        inner_rect_dim = 40

        defs_g.add(dwg.rect((0, 0), (dim, dim),
                            fill=self.color1))
        defs_g.add(dwg.rect(((dim-rotated_rect_dim)//2, (dim-rotated_rect_dim)//2),
                             (rotated_rect_dim, rotated_rect_dim),
                            fill=self.color2,
                            transform="rotate(45,"+str(dim//2)+","+str(dim//2)+")"))
        defs_g.add(dwg.circle((dim//2, -R_offset), R,
                              fill=self.color1))
        defs_g.add(dwg.circle((-R_offset, dim//2), R,
                              fill=self.color1))
        defs_g.add(dwg.circle((dim+R_offset, dim//2), R,
                              fill=self.color1))
        defs_g.add(dwg.circle((dim//2, dim+R_offset), R,
                              fill=self.color1))
        defs_g.add(dwg.rect(((dim-inner_rect_dim)//2, (dim-inner_rect_dim)//2),
                             (inner_rect_dim, inner_rect_dim),
                            fill=self.color1))

        dwg.add(dwg.use(defs_g, insert=(0, 0)))
        dwg.save()

class CircleSubtractPattern3:
    def __init__(self, color1, color2):
        self.color1 = color1
        self.color2 = color2

    def save(self, filename):
        dwg = svgwrite.Drawing(filename, size=('120', '120'))
        clip_path = dwg.defs.add(dwg.clipPath(id='clipsq'))
        clip_path.add(dwg.rect((0, 0), (120, 120)))
        defs_g = dwg.defs.add(dwg.g(clip_path='url(#clipsq)'))

        R=20
        b=20
        q=1.5

        defs_g.add(dwg.rect((0, 0), (120, 120), fill=self.color1))
        points = (((120-q*R)//2,b),
                  ((120+q*R)//2,b),
                  (120-b,(120-q*R)//2),
                  (120-b,(120+q*R)//2),
                  ((120+q*R)//2,120-b),
                  ((120-q*R)//2,120-b),
                  (b,(120+q*R)//2),
                  (b,(120-q*R)//2))

        defs_g.add(dwg.polygon(points, fill=self.color2))
        defs_g.add(dwg.circle((60, 60), R, fill=self.color1))

        dwg.add(dwg.use(defs_g, insert=(0, 0)))
        dwg.save()

class CircleSubtractPattern4:
    def __init__(self, color1, color2):
        self.color1 = color1
        self.color2 = color2

    def save(self, filename):
        dwg = svgwrite.Drawing(filename, size=('120', '120'))
        clip_path = dwg.defs.add(dwg.clipPath(id='clipsq'))
        clip_path.add(dwg.rect((0, 0), (120, 120)))
        defs_g = dwg.defs.add(dwg.g(clip_path='url(#clipsq)'))

        R = 38
        r = 14
        r_offset = 32

        defs_g.add(dwg.rect((0, 0), (120, 120), fill=self.color1))
        defs_g.add(dwg.circle((60, 60), R, fill=self.color2))
        defs_g.add(dwg.circle((r_offset, r_offset), r, fill=self.color2))
        defs_g.add(dwg.circle((120-r_offset, 30), r, fill=self.color2))
        defs_g.add(dwg.circle((30, 120-r_offset), r, fill=self.color2))
        defs_g.add(dwg.circle((120-r_offset, 120-r_offset), r, fill=self.color2))
        points = ((35,35),
                  (60, 40),
                  (85, 35),
                  (80, 60),
                  (85, 85),
                  (60, 80),
                  (35, 85),
                  (40, 60))

        defs_g.add(dwg.polygon(points, fill=self.color1))
        dwg.add(dwg.use(defs_g, insert=(0, 0)))
        dwg.save()


#yellow = 'rgb(230,210,16)'
yellow = 'rgb(255,255,44)'
green = 'rgb(31,161,97)'
#darkblue = 'rgb(49,105,142)'
darkblue = 'rgb(36,71,115)'
blue = 'rgb(74,196,231)'
#lightblue = 'rgb(74,196,231)'
lightblue = 'rgb(172,225,238)'
pink = 'rgb(216,96,137)'
#purple = 'rgb(103,46,91)'
purple = 'rgb(99,0,132)'
tapestry = 'rgb(163,84,105)'
orange = 'orange'

CircleSubtractPattern(orange, lightblue).save('pattern1.svg')
CircleSubtractPattern2(pink, lightblue).save('pattern2.svg')
CircleSubtractPattern3(green, darkblue).save('pattern3.svg')
FlowerPattern(darkblue, yellow).save('pattern4.svg')
CircleSubtractPattern4(tapestry, orange).save('pattern5.svg')
RoundCrossPattern(pink, yellow).save('pattern6.svg')
CrossPattern(purple, lightblue).save('pattern7.svg')
PlusPattern(purple, yellow).save('pattern8.svg')
CrossPattern(green, orange).save('pattern9.svg')
MultiSquarePattern(blue, pink).save('pattern10.svg')
SquarePattern(yellow, green).save('pattern11.svg')
RoundCrossPattern(darkblue, orange).save('pattern12.svg')
MultiSquarePattern(pink, yellow).save('pattern13.svg')
StarPattern(yellow, blue).save('pattern14.svg')
RoundCrossPattern(green, pink).save('pattern15.svg')
StarPattern(tapestry, yellow).save('pattern16.svg')
CrossPattern(blue, pink).save('pattern17.svg')
MultiSquarePattern(yellow, darkblue).save('pattern18.svg')
StarPattern(orange, tapestry).save('pattern19.svg')
SquarePattern(darkblue, lightblue).save('pattern20.svg')
PlusPattern(darkblue, pink).save('pattern21.svg')
PlusPattern(tapestry, green).save('pattern22.svg')

