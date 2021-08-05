import java.awt.*;
import java.awt.event.*;

// javac -cp "sandmark.jar" -Xlint:deprecation  TTT.java
// jar cfe TTT.jar TTT TTT.class

public class TTT extends Frame {
    Label lab = new Label("Tic-Tac-Toe", Label.CENTER);
    Panel pan = new Panel();
    Button[] sq = new Button[9];
    Panel south = new Panel();
    Button quit = new Button("Exit");
    Button reload = new Button("Reload");
    int ply = 2;
    public void start() {
        for (int p = 0; p < 9; p++) {
            sq[p] = new Button(".");
            sq[p].setActionCommand(Integer.toString(p));
            sq[p].addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    int b = new Integer(e.getActionCommand()).intValue();
                    move(b);
                }
            });
            pan.add(sq[p]);
        }
    }
    public void init() {
        sandmark.watermark.ct.trace.Annotator.sm$mark();
        setBackground(Color.green);
        setForeground(Color.yellow);
        setFont(new Font("SansSerif", Font.BOLD, 60));
        setSize(360, 360);
        setLayout(new BorderLayout());
        addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent e) {
                dispose();
                System.exit(0);
            }
        });
        add("North", lab);
        add("Center", pan);
        pan.setLayout(new GridLayout(3, 3));
        sandmark.watermark.ct.trace.Annotator.sm$mark();
        south.setLayout(new FlowLayout());
        quit.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                dispose();
                System.exit(0);
            }
        });
        reload.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                clear();
            }
        });
        add("South", south);
        south.add(quit);
        south.add(reload);
    }
    public static void main(String[] args) {
        TTT ttt = new TTT();
        ttt.init();
        ttt.start();
        ttt.pack();
        ttt.setVisible(true);
    }
    public void clear() {
        setForeground(Color.yellow);
        for (int p = 0; p < 9; p++)
            sq[p].setLabel(".");
        sandmark.watermark.ct.trace.Annotator.sm$mark();
        lab.setText("Tic-Tac-Toe");
    }
    public void move(int b) {
        if (!hit() && free()) {
            ply = (ply == 2) ? 1 : 2;
            sandmark.watermark.ct.trace.Annotator.sm$mark(ply);
            mark(b);
            if (hit()) {
                setForeground(Color.blue);
                lab.setText("Player " + ply + " won!");
            }
        } else {
            if (free()) setForeground(Color.magenta);
            else setForeground(Color.red);
            lab.setText("Reload game!");
        }
    }
    public boolean free() {
        boolean f = false;
        for (int b = 0; b < 9; b++)
            if (sq[b].getLabel() == ".") f = true;
        return f;
    }
    public boolean hit() {
        boolean hit = singleHit(0, 4, 8) | singleHit(2, 4, 6);
        for (int p = 0; p < 3; p++) hit |= singleHit(p, p + 3, p + 6);
        for (int p = 0; p <9; p += 3) hit |= singleHit(p, p + 1, p + 2);
        return hit;
    }
    public void mark(int b) {
        if (sq[b].getLabel() == ".") {
            if (ply == 1)
                sq[b].setLabel("X");
            else
                sq[b].setLabel("O");
            sandmark.watermark.ct.trace.Annotator.sm$mark(b);
        }
    }
    private boolean singleHit(int b1, int b2, int b3) {
        return ((sq[b1].getLabel() != ".") &&
            (sq[b1].getLabel() == sq[b2].getLabel()) &&
            (sq[b1].getLabel() == sq[b3].getLabel()));
    }
}
