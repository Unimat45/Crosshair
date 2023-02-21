namespace Pointer {
    using VK = Externs.VK;
    public partial class Overlay : Form {
        private Graphics? g;
        private readonly Pen pen;
        private readonly Pen penBlack;

        private float offsetX = 0f;
        private float offsetY = 0f;

        public Overlay() {
            InitializeComponent();
            pen = new(Color.White, .5f);
            penBlack = new(Color.Black, .5f);

            this.ShowInTaskbar = false;

            new Task(() => {
                while (true) {
                    bool isUpOn     =   Externs.IsKeyDown(VK.UP);
                    bool isDownOn   =   Externs.IsKeyDown(VK.DOWN);
                    bool isLeftOn   =   Externs.IsKeyDown(VK.LEFT);
                    bool isRightOn  =   Externs.IsKeyDown(VK.RIGHT);

                    bool isAltOn    =   Externs.IsKeyDown(VK.RMENU);
                    bool isHomeOn   =   Externs.IsKeyDown(VK.HOME);

                    if (isAltOn) {

                        if (isHomeOn) {
                            offsetX = 0f;
                            offsetY = 0f;
                        }

                        if (isLeftOn)   { offsetX--; }
                        if (isRightOn)  { offsetX++; }
                        if (isUpOn)     { offsetY--; }
                        if (isDownOn)   { offsetY++; }
                    }

                    Thread.Sleep(50);
                }
            }).Start();
        }

        private void Overlay_Load(object sender, EventArgs e) {
            this.BackColor = Color.Wheat;
            this.TransparencyKey = Color.Wheat;
            this.FormBorderStyle = FormBorderStyle.None;

            this.TopLevel = true;
            this.TopMost = true;

            int initialStyle = Externs.GetWindowLong(this.Handle, -20);
            Externs.SetWindowLong(this.Handle, -20, initialStyle | 0x80000 | 0x20);

            this.PositionWindow();
            new Task(Prepaint).Start();
        }

        public void Prepaint() {
            while (true) {
                this.PositionWindow();
                this.SafeExec(form => form.Refresh());

                Thread.Sleep(50); // Refresh rate => 50ms
            }
        }

        private void PositionWindow() {
            if (this.InvokeRequired) {
                this.Invoke(delegate { PositionWindow(); });
            }
            else {
                Rectangle rect = Screen.FromControl(this).Bounds;

                this.Size = new Size(rect.Right - rect.Left, rect.Bottom - rect.Top);

                this.Top = rect.Top;
                this.Left = rect.Left;

                this.TopMost = true;
                this.TopLevel = true;
            }
        }

        private void Overlay_Paint(object sender, PaintEventArgs e) {
            g = e.Graphics;

            float width = this.Width / 2 + offsetX;
            float height = this.Height / 2 + offsetY;

            float multi = Form1.ballSize;
            // UP DOWN
            g.DrawLine(pen, width - multi, height, width + multi, height);

            g.DrawLine(pen, width, height - multi, width, height + multi);


            float widthBlack = this.Width / 2 + 1 + offsetX;
            float heightBlack = this.Height / 2 + 1 + offsetY;

            // BLACK
            g.DrawLine(penBlack, widthBlack - multi, heightBlack, widthBlack + multi, heightBlack);

            g.DrawLine(penBlack, widthBlack, heightBlack - multi, widthBlack, heightBlack + multi);
        }
    }

    internal static class ControlExt {
        public static void SafeExec<T>(this T control, Action<T> cb) where T : Control {
            if (control.IsDisposed) return;

            if (control.InvokeRequired) {
                control.Invoke(delegate { SafeExec(control, cb); });
            }
            else {
                cb(control);
            }
        }
    }
}
