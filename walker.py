import tree
import enum
from collections import deque

auto = (i for i in range(256))

class Instr:
    class Kind(enum.IntEnum):
        INIT = next(auto)
        PUSH = next(auto)
        POP = next(auto)
        CALL = next(auto)
        STORE = next(auto)
        LOAD = next(auto)
        FUNC = next(auto)
        CAPTURE = next(auto)
        RET = next(auto)
        NAME = next(auto)
        SPACE = next(auto)
        JUMP = next(auto)
        JUMPFALSE = next(auto)

    def __init__(self, kind, *vals):
        self.kind = kind
        self.vals = deque(vals)
    def __str__(self):
        name = str(self.kind)[5:].lower()
        if name == 'push':
            if isinstance(self.vals[0], str):
                return 'str' + ' '*13 + str(self.vals[0])
        return name + ' '*(16-len(name)) + ' '.join(map(str, self.vals))
    def __repr__(self):
        return str(self)

class Walker:
    def __init__(self):
        self.code = []
        self.places = [[1, 1]]
        self.locals = []
        self.ctx = []
    def emit(self, pack_opcode, *pack_values):
        self.code.append(Instr(pack_opcode, *pack_values))
        self.places.append(self.places[-1])
    def local_where(self, name):
        ret = self.ctx[-1].all_locals.index(name)
        return ret
    def ctx_enter(self, node):
        self.ctx.append(node)
        self.locals.append(node.need_cap)
    def ctx_exit(self):
        self.locals.pop()
        self.ctx.pop()
    def walk_root(self, node):
        self.emit(Instr.Kind.INIT)
        self.ctx_enter(node)
        for i in node.all_locals:
            if i not in node.need_cap:
                self.emit(Instr.Kind.SPACE)
            else:
                self.emit(Instr.Kind.NAME, i)                
        for i in node.code:
            self.walk(i)
            self.emit(Instr.Kind.POP)
        self.ctx_exit()
    def walk_if(self, node):
        self.walk(node.cond)
        begin = len(self.code)
        self.emit(Instr.Kind.JUMPFALSE)
        self.walk(node.true)
        iffalse = len(self.code)
        self.emit(Instr.Kind.JUMP)
        if node.false is None:
            self.emit(Instr.Kind.PUSH, None)
        else:
            self.walk(node.false)
        self.code[begin].vals.append(iffalse)
        self.code[iffalse].vals.append(len(self.code)-1)
    def walk_call(self, node):
        self.walk(node.pack_func)
        for i in node.args:
            self.walk(i)
        self.emit(Instr.Kind.CALL, len(node.args))
    def walk_load(self, node):
        index = self.local_where(node.name)
        self.emit(Instr.Kind.LOAD, index)
        self.places[-1] = node.place
    def walk_const(self, node):
        self.emit(Instr.Kind.PUSH, node.val)
        self.places[-1] = node.place
    def walk_lambda(self, node):
        for i in node.need_cap:
            self.emit(Instr.Kind.CAPTURE, self.local_where(i))
        place = len(self.code)
        self.emit(Instr.Kind.FUNC)
        self.ctx_enter(node)
        for i in node.all_locals:
            if i in node.argnames:
                pass
            elif i in node.need_cap:
                pass
            else:
                self.emit(Instr.Kind.SPACE)
        for i in node.code:
            self.walk(i)
            self.emit(Instr.Kind.POP)
        if len(node.code) == 0:
            self.emit(Instr.Kind.PUSH, None)
        else:
            self.code.pop()
        self.code[place].vals.append(len(self.code))
        self.emit(Instr.Kind.RET)
        self.ctx_exit()
    def walk_define(self, node):
        if node.is_pack_func:
            for i in node.need_cap:
                self.emit(Instr.Kind.CAPTURE, self.local_where(i))
            place = len(self.code)
            self.emit(Instr.Kind.FUNC)
            self.places[-1] = node.place
            self.ctx_enter(node)
            for i in node.all_locals:
                if i in node.argnames:
                    pass
                elif i in node.need_cap:
                    pass
                else:
                    self.emit(Instr.Kind.SPACE)
            for i in node.val:
                self.walk(i)
                self.emit(Instr.Kind.POP)
            if len(node.val) == 0:
                self.emit(Instr.Kind.PUSH, None)
            else:
                self.code.pop()
            self.code[place].vals.append(len(self.code))
            self.emit(Instr.Kind.RET)
            self.ctx_exit()
        else:
            self.walk(node.val)
        self.emit(Instr.Kind.STORE, self.local_where(node.name))
        self.places[-1] = node.place
    def walk(self, node):
        if isinstance(node, tree.Root):
            self.walk_root(node)
        elif isinstance(node, tree.Define):
            self.walk_define(node)
        elif isinstance(node, tree.Lambda):
            self.walk_lambda(node)
        elif isinstance(node, tree.Call):
            self.walk_call(node)
        elif isinstance(node, tree.Load):
            self.walk_load(node)
        elif isinstance(node, tree.Const):
            self.walk_const(node)
        elif isinstance(node, tree.If):
            self.walk_if(node)
        else:
            raise Exception('not implented')