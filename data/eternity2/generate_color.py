import svgwrite

DIM = 120

class SquarePattern:
    def __init__(self, color1, color2):
        self.color1 = color1
        self.color2 = color2

    def save(self, filename):
        dwg = svgwrite.Drawing(filename)
        dwg.add(dwg.rect((0, 0), (DIM, DIM), fill=self.color1))
        dwg.add(dwg.rect((20, 20), (80, 80), fill=self.color2))
        dwg.add(dwg.rect((40, 40), (40, 40), fill=self.color1))
        dwg.save()

class MultiSquarePattern:
    def __init__(self, color1, color2):
        self.color1 = color1
        self.color2 = color2

    def save(self, filename):
        dwg = svgwrite.Drawing(filename)
        dwg.add(dwg.rect((0, 0), (120, 120), fill=self.color1))
        dwg.add(dwg.rect((30, 30), (60, 60), fill=self.color2))
        dwg.add(dwg.rect((15, 15), (30, 30), fill=self.color2))
        dwg.add(dwg.rect((75, 15), (30, 30), fill=self.color2))
        dwg.add(dwg.rect((15, 75), (30, 30), fill=self.color2))
        dwg.add(dwg.rect((75, 75), (30, 30), fill=self.color2))
        dwg.save()

class PlusPattern:
    def __init__(self, color1, color2):
        self.color1 = color1
        self.color2 = color2

    def save(self, filename):
        dwg = svgwrite.Drawing(filename)
        dwg.add(dwg.rect((0, 0), (120, 120), fill=self.color1))
        dwg.add(dwg.circle((60, 60), 50, fill=self.color2))
        dwg.add(dwg.rect((50, 30), (20, 60), fill=self.color1))
        dwg.add(dwg.rect((30, 50), (60, 20), fill=self.color1))
        dwg.save()

class RoundCrossPattern:
    def __init__(self, color1, color2):
        self.color1 = color1
        self.color2 = color2

    def save(self, filename):
        dwg = svgwrite.Drawing(filename)
        dwg.add(dwg.rect((0, 0), (120, 120), fill=self.color1))

        dwg.add(dwg.circle((30, 30), 18, fill=self.color2))
        dwg.add(dwg.circle((90, 30), 18, fill=self.color2))
        dwg.add(dwg.circle((30, 90), 18, fill=self.color2))
        dwg.add(dwg.circle((90, 90), 18, fill=self.color2))
        dwg.add(dwg.rect((20, 47.5), (80, 25), fill=self.color2, transform="rotate(45,60,60)"))
        dwg.add(dwg.rect((20, 47.5), (80, 25), fill=self.color2, transform="rotate(-45,60,60)"))
        dwg.save()

class CrossPattern:
    def __init__(self, color1, color2):
        self.color1 = color1
        self.color2 = color2

    def save(self, filename):
        dwg = svgwrite.Drawing(filename)
        dwg.add(dwg.rect((0, 0), (120, 120), fill=self.color1))

        points = ((15,15),(45,15),(60, 45),(60, 60),(45, 60),(15,45))
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
        dwg = svgwrite.Drawing(filename)
        dwg.add(dwg.rect((0, 0), (120, 120), fill=self.color1))

        R = 52
        r = 28
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
        dwg = svgwrite.Drawing(filename)
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
        dwg = svgwrite.Drawing(filename)
        dwg.add(dwg.rect((0, 0), (120, 120), fill=self.color1))
        dwg.add(dwg.circle((40, 40), 23, fill=self.color2))
        dwg.add(dwg.circle((80, 40), 23, fill=self.color2))
        dwg.add(dwg.circle((40, 80), 23, fill=self.color2))
        dwg.add(dwg.circle((80, 80), 23, fill=self.color2))
        dwg.add(dwg.circle((60, 60), 23, fill=self.color1))
        dwg.save()

class CircleSubtractPattern2:
    def __init__(self, color1, color2):
        self.color1 = color1
        self.color2 = color2

    def save(self, filename):
        dwg = svgwrite.Drawing(filename)
        clip_path = dwg.defs.add(dwg.clipPath(id='clipsq'))
        clip_path.add(dwg.rect((0, 0), (120, 120)))
        defs_g = dwg.defs.add(dwg.g(clip_path='url(#clipsq)'))

        defs_g.add(dwg.rect((0, 0), (120, 120), fill=self.color1))
        defs_g.add(dwg.rect((5, 5), (110, 110), fill=self.color2, transform="rotate(45,60,60)"))
        defs_g.add(dwg.circle((60, -10), 30, fill=self.color1))
        defs_g.add(dwg.circle((-10, 60), 30, fill=self.color1))
        defs_g.add(dwg.circle((130, 60), 30, fill=self.color1))
        defs_g.add(dwg.circle((60, 130), 30, fill=self.color1))
        defs_g.add(dwg.rect((40, 40), (40, 40), fill=self.color1))

        dwg.add(dwg.use(defs_g, insert=(0, 0)))
        dwg.save()

class CircleSubtractPattern3:
    def __init__(self, color1, color2):
        self.color1 = color1
        self.color2 = color2

    def save(self, filename):
        dwg = svgwrite.Drawing(filename)
        clip_path = dwg.defs.add(dwg.clipPath(id='clipsq'))
        clip_path.add(dwg.rect((0, 0), (120, 120)))
        defs_g = dwg.defs.add(dwg.g(clip_path='url(#clipsq)'))

        R=22
        b=15
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
        dwg = svgwrite.Drawing(filename)
        clip_path = dwg.defs.add(dwg.clipPath(id='clipsq'))
        clip_path.add(dwg.rect((0, 0), (120, 120)))
        defs_g = dwg.defs.add(dwg.g(clip_path='url(#clipsq)'))

        defs_g.add(dwg.rect((0, 0), (120, 120), fill=self.color1))
        defs_g.add(dwg.circle((60, 60), 40, fill=self.color2))
        defs_g.add(dwg.circle((30, 30), 16, fill=self.color2))
        defs_g.add(dwg.circle((90, 30), 16, fill=self.color2))
        defs_g.add(dwg.circle((30, 90), 16, fill=self.color2))
        defs_g.add(dwg.circle((90, 90), 16, fill=self.color2))
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


yellow = 'rgb(230,210,16)'
green = 'rgb(31,161,97)'
darkblue = 'rgb(49,105,142)'
lightblue = 'rgb(74,196,231)'
pink = 'rgb(216,96,137)'
purple = 'rgb(103,46,91)'
tapestry = 'rgb(163,84,105)'
orange = 'orange'

CircleSubtractPattern(orange, lightblue).save('color1.svg')
CircleSubtractPattern2(pink, lightblue).save('color2.svg')
CircleSubtractPattern3(green, darkblue).save('color3.svg')
FlowerPattern(darkblue, yellow).save('color4.svg')
CircleSubtractPattern4(tapestry, orange).save('color5.svg')
RoundCrossPattern(pink, yellow).save('color6.svg')
CrossPattern(purple, lightblue).save('color7.svg')
PlusPattern(purple, yellow).save('color8.svg')
CrossPattern(green, orange).save('color9.svg')
MultiSquarePattern(lightblue, pink).save('color10.svg')
SquarePattern(yellow, green).save('color11.svg')
RoundCrossPattern(darkblue, orange).save('color12.svg')
MultiSquarePattern(pink, yellow).save('color13.svg')
StarPattern(yellow, lightblue).save('color14.svg')
RoundCrossPattern(green, pink).save('color15.svg')
StarPattern(tapestry, yellow).save('color16.svg')
CrossPattern(lightblue, pink).save('color17.svg')
MultiSquarePattern(yellow, darkblue).save('color18.svg')
StarPattern(orange, tapestry).save('color19.svg')
SquarePattern(darkblue, lightblue).save('color20.svg')
PlusPattern(darkblue, pink).save('color21.svg')
PlusPattern(tapestry, green).save('color22.svg')

