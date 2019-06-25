import Sys;
import String;
class Main {
    static public function main():Void {
        var args = Sys.args();
        var interp = new Backend.Vm();
        interp.runfile(args[0]);
        // var parsed = new Lisp.Parser('(+ 2 2)');
        // var em = Emit.Emmiter();
        // Sys.println(parsed.root);
    }
}