import pygame

GRAY = (53, 87, 100)#(192, 192, 192)
EMPTY_GRAY = (192, 192, 192)#(120, 120, 120)
WHITE = (255, 255, 255)
BLUE = (0, 0, 255)


class BoardUi:

    def __init__(self, board):
        self.board = board
        self.piece_width = 50
        self.font = None
        self.piece_img = {}
        self.empty_img = None
        self.marks_enabled = False
        self.DISPLAY = None

    def init(self):
        pygame.init()
        pygame.font.init()  # you have to call this at the start,
        # if you want to use this module.
        self.font = pygame.font.SysFont('Comic Sans MS', 12, bold=True)

        pygame.display.set_caption('Puzzle')
        # make the piece width dynamic to fill about the same portion of screen always
        max_dim = max(self.board.puzzle_def.width, self.board.puzzle_def.height)
        self.piece_width = ((800//max_dim)//4)*4

        self.DISPLAY = pygame.display.set_mode((self.piece_width * self.board.puzzle_def.width,
                                                self.piece_width * self.board.puzzle_def.height))
        self.DISPLAY.fill(WHITE)

        # load color images
        color_images = {}
        for i in range(1, 22 + 1):
            color_images[i] = pygame.image.load(r'data\patterns\pattern%i.png' % i)

        # construct individual piece images
        color_dim = color_images[1].get_height()
        for id, piece in self.board.puzzle_def.all.items():
            self.piece_img[id] = []
            high_res = pygame.Surface((2 * color_dim, 2 * color_dim))
            high_res.fill(GRAY)
            color_id = piece.colors[2]
            if color_id != 0:
                high_res.blit(pygame.transform.rotate(
                    color_images[color_id],
                    0),
                    (0, 0))
            color_id = piece.colors[3]
            if color_id != 0:
                high_res.blit(pygame.transform.rotate(
                    color_images[color_id],
                    270),
                    (color_dim, 0))
            color_id = piece.colors[1]
            if color_id != 0:
                high_res.blit(pygame.transform.rotate(
                    color_images[color_id],
                    90),
                    (0, color_dim))
            color_id = piece.colors[0]
            if color_id != 0:
                high_res.blit(pygame.transform.rotate(
                    color_images[color_id],
                    180),
                    (color_dim, color_dim))

            for dir in range(4):
                high_res2 = pygame.transform.rotate(high_res, 45 - dir * 90)

                h = high_res2.get_height()/4
                low_res = pygame.Surface((2 * h, 2 * h))
                low_res.blit(high_res2, (0, 0), (h, h, 2 * h, 2 * h))
                low_res = pygame.transform.scale(low_res, (self.piece_width, self.piece_width))
                pygame.draw.lines(low_res, (50, 50, 50), True, [(0,0),(self.piece_width, self.piece_width)], 2)
                pygame.draw.lines(low_res, (50, 50, 50), True, [(self.piece_width, 0), (0, self.piece_width)], 2)
                self.piece_img[id].append(low_res)

        # empty field
        self.empty_img = pygame.Surface((self.piece_width, self.piece_width))
        self.empty_img.fill(EMPTY_GRAY)

        pygame.display.update()
        pygame.display.flip()
        self.update()
        pass

    def draw(self, piece, x, y):
        border_points = [(0 + x, 0 + y),
                         (0 + x, self.piece_width + y),
                         (self.piece_width + x, self.piece_width + y),
                         (self.piece_width + x, 0 + y)]

        def draw_border():
            pygame.draw.lines(self.DISPLAY, (50, 50, 50), True, border_points, 2)

        if piece:
            i, j = piece.i, piece.j
            id = piece.piece_def.id
            self.DISPLAY.blit(self.piece_img[id][piece.dir], (x, y))
            draw_border()
            if self.board.marks[i][j] and self.marks_enabled:
                textsurface = self.font.render(str(self.board.marks[i][j]), False, (255, 255, 255))
                text_rect = textsurface.get_rect(center=(x + self.piece_width // 2, y + self.piece_width // 2))

                s = pygame.Surface((25, 20))  # the size of your rect
                s.set_alpha(100)  # alpha level
                s.fill((50, 50, 50))  # this fills the entire surface
                centered = s.get_rect(center=(x + self.piece_width // 2, y + self.piece_width // 2))
                self.DISPLAY.blit(s, centered)  # (0,0) are the top-left coordinates

                self.DISPLAY.blit(textsurface, text_rect)
        else:
            self.DISPLAY.blit(self.empty_img, (x, y))
            draw_border()

    def update(self):
        for i in range(self.board.puzzle_def.height):
            for j in range(self.board.puzzle_def.width):
                self.draw(self.board.board[i][j], self.piece_width * j, self.piece_width * i)
