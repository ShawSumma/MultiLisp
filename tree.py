import lark

class Capture:
    def __init__(self, items=None):
        if items == None:
            self.items = []
        else:
            self.items = list(items)
        self.locals = []
    def add(self, item):
        if item not in self.items:
            self.items.append(item)
    def update(self, other):
        if isinstance(other, set):
            self.items += other
        elif isinstance(other, Capture):
            self.items += other.get_cap()
        else:
            raise Exception("error")
    def local(self, item):
        if item not in self.locals:
            self.locals.append(item)
    def get_cap(self):
        return [i for i in self.items if i not in self.locals]
    def get_names(self):
        ret = list(self.items)
        for i in self.locals:
            if i not in ret:
                ret.append(i)
        return ret

class Node:
    def get(self):
        vs = vars(self)
        for i in vs:
            item = vs[i]
            if isinstance(item, Node):
                got = vs[i].get()
            elif isinstance(item, (list, tuple)):
                got = [j.get() if isinstance(j, Node) else j for j in item]
            else:
                got = item
            setattr(self, i, got)
        return self
    def walk(self, depth=0):
        vs = vars(self)
        print('  '*depth + self.__class__.__name__)
        for i in vs:
            item = vs[i]
            if isinstance(item, Node):
                vs[i].walk(depth+1)
            elif isinstance(item, (str, bool)):
                print('  '*(depth+1) + i + ":", item)
            else:
                print('  '*(depth+1) + i + ": ", end='')
                self.walk_ext(item, depth+1)
        return self
    def walk_ext(self, item, depth):
        if isinstance(item, (str, bool, float, int)) or item is None:
            print('  ' + str(item))
        elif len(item) == 0:
             print("[]")                
        else:
            print("[")
            for i in item:
                if isinstance(i, Node):
                    i.walk(depth+1)
                else:
                    print('  '*depth, end='')
                    self.walk_ext(i, depth+1)
            print('  '*(depth) + "]")
    def capture(self, node, ctx):
        node.captures(ctx)
    def captures(self, ctx=None):
        pass

class Root(Node):
    def __init__(self, code):
        self.code = code
        self.need_cap = None
        self.all_locals = None
    def captures(self, ctx=None):
        ctx = Capture()
        for i in self.code:
            self.capture(i, ctx)
        self.need_cap = ctx.get_cap().copy()
        self.all_locals = ctx.get_names().copy()

class Define(Node):
    def __init__(self, name, *code):
        self.is_func = isinstance(name, Call)
        self.place = list(name.func.place)
        if self.is_func:
            self.name = str(name.func)
            self.argnames = []
            for i in name.args:
                self.argnames.append(str(i))
            self.val = code      
        else:
            self.name = str(name)
            self.val = code[0]
        self.need_cap = None
        self.all_locals = None
    def captures(self, ctx):
        ctx.local(self.name)
        if self.is_func:
            # ctx.add('define')
            new_ctx = Capture()
            # new_ctx.local(self.name)
            for i in self.argnames:
                new_ctx.local(str(i))
                new_ctx.add(str(i))
            for i in self.val:
                self.capture(i, new_ctx)
            self.need_cap = new_ctx.get_cap().copy()
            self.all_locals = new_ctx.get_names().copy()
            ctx.update(new_ctx)
        else:
            self.capture(self.val, ctx)
    def get(self):
        if self.is_func:
            val = []
            for i in self.val:
                val.append(i.get())
            self.val = val
        else:
            self.val = self.val.get()
        return self

class If(Node):
    def __init__(self, cond, true, false=None):
        self.cond = cond
        self.true = true
        self.false = false
    def captures(self, ctx):
        self.capture(self.cond, ctx)
        self.capture(self.true, ctx)
        if self.false is not None:
            self.capture(self.false, ctx)
    def get(self):
        self.cond = self.cond.get()
        self.true = self.true.get()
        if self.false is not None:
            self.false = self.false.get()
        return self

class Lambda(Node):
    def __init__(self, args, *code):
        self.argnames= []
        if args.func is not None:
            self.argnames.append(str(args.func))
        for i in args.args:
            self.argnames.append(str(i))
        self.code = code
        self.need_cap = None
        self.all_locals = None
    def captures(self, ctx):
        new_ctx = Capture()
        for i in self.argnames:
            new_ctx.local(str(i))
            new_ctx.add(str(i))
        for i in self.code:
            self.capture(i, new_ctx)
        ctx.update(new_ctx)
        self.need_cap = new_ctx.get_cap().copy()
        self.all_locals = new_ctx.get_names().copy()
    def get(self):
        code = []
        for i in self.code:
            code.append(i.get())
        self.code = code
        return self

class Call(Node):
    def __init__(self, func=None, *args):
        self.func = func
        self.args = args
    def is_empty(self):
        return self.func is not None
    def is_namecall(self):
        return isinstance(self.func, Load)
    def is_calling(self, name):
        return self.is_namecall() and self.func == name
    def get(self):
        if self.is_calling('define'):
            return Define(*self.args).get()
        elif self.is_calling('lambda'):
            return Lambda(*self.args).get()
        elif self.is_calling('if'):
            return If(*self.args).get()
        else:
            self.func = self.func.get()
            args = []
            for i in self.args:
                args.append(i.get())
            self.args = args
            return self
    def captures(self, ctx):
        self.capture(self.func, ctx)
        for i in self.args:
            self.capture(i, ctx)

class Load(Node):
    def __init__(self, name, place=None):
        self.name = name
        self.place = place
    def __eq__(self, other):
        if isinstance(other, str):
            return str(self) == other
        if isinstance(other, Load):
            return other.name == self.name
        return False
    def __str__(self):
        return self.name
    def captures(self, ctx):
        ctx.add(self.name)

class Const(Node):
    def __init__(self, val, place=None):
        self.place = place
        if isinstance(val, float) and val % 1 == 0:
            self.val = int(val)
        else:
            self.val = val
    
class State:
    def __init__(self):
        pass
    def parse(self, code):
        with open('simple.lark') as f:
            parser = lark.Lark(f.read())
        ast = parser.parse(code)
        return ast
    def change(self, ast):
        if isinstance(ast, lark.Tree):
            if ast.data == 'start':
                code = []
                for i in ast.children:
                    code.append(self.change(i))
                return Root(code)
            else:
                code = []
                for i in ast.children:
                    code.append(self.change(i))
                return Call(*code)
        else:
            place = (ast.line, ast.column)
            if ast.type == 'NAME':
                return Load(str(ast), place=place)
            if ast.type == 'NUM':
                return Const(float(str(ast)), place=place)
            if ast.type == 'STR':
                return Const(str(ast)[1:-1], place=place)