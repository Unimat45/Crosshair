namespace Pointer {
    using VK = Externs.VK;
    public partial class Overlay : Form {
        // Refresh rate of the application in miliseconds
        private const int REFRESH_RATE = 50;

        // Used to draw the lines
        private Graphics? g;
        private readonly Pen pen;
        private readonly Pen penBlack;

        // Default config, used if no config is loaded (Created in this file because the screen width and height are required)
        private readonly Config defaultCfg;

        // If the crosshair is hidden
        private bool isHidden = false;

        // Crosshair offsets, used to move crosshair with arrows
        private float offsetX = 0f;
        private float offsetY = 0f;

        // Width of lines constant
        private const float multi = 5f;

        public Overlay() {
            InitializeComponent();
            
            // Pen initialization
            pen = new(Color.White, .5f);
            penBlack = new(Color.Black, .5f);

            // TODO: Find out why this don't work 😥
            // System Tray icon
            NotifyIcon icon = new() {
                Text = "Pointer",
                Visible = true,
                Icon = new Icon("generic.ico")
            };

            ContextMenuStrip strip = new();
             
            ToolStripMenuItem item = new() {
                Text = "Exit"
            };
            item.Click += new EventHandler((object? sender, EventArgs args) => this.Close());

            strip.Items.Add(item);
            icon.ContextMenuStrip = strip;

            // Positions window to fit whole screen
            this.PositionWindow();

            Config.LoadFromFile(this.Width, this.Height);
            
            // Creates a default configuration with rules to put the crosshair in the middle of the screen
            defaultCfg = new Config(this.Width, this.Height, "None", "%w / 2", "%h / 2");

            this.ShowInTaskbar = false;

            // Keyboard shortcut Thread
            new Task(() => {
                while (true) {
                    bool isUpOn     =   Externs.IsKeyDown(VK.UP);
                    bool isPDown    =   Externs.IsKeyDown(VK.KEY_P);
                    bool isQDown    =   Externs.IsKeyDown(VK.KEY_Q);
                    bool isDownOn   =   Externs.IsKeyDown(VK.DOWN);
                    bool isLeftOn   =   Externs.IsKeyDown(VK.LEFT);
                    bool isRightOn  =   Externs.IsKeyDown(VK.RIGHT);

                    bool isShiftOn  =   Externs.IsKeyDown(VK.LSHIFT);
                    bool isCtrlOn   =   Externs.IsKeyDown(VK.CONTROL);
                    bool isAltOn    =   Externs.IsKeyDown(VK.RMENU);
                    bool isLeftAltOn=   Externs.IsKeyDown(VK.LMENU);
                    bool isHomeOn   =   Externs.IsKeyDown(VK.HOME);

                    // Quitting Sequence
                    if (isLeftAltOn && isShiftOn && isCtrlOn && isQDown) {
                        this.SafeExec(form => form.Close());
                    }


                    if (isAltOn) {
                        // Toggling sequence
                        if (isCtrlOn && isPDown) {
                            if (isHidden) {
                                this.SafeExec(form => Show());
                            }
                            else {
                                this.SafeExec(form => form.Hide());
                            }
                            this.isHidden = !this.isHidden;

                            // Added for comfort of usage
                            Thread.Sleep(100);
                        }

                        // Resets to crosshair's position
                        if (isHomeOn) {
                            offsetX = 0f;
                            offsetY = 0f;
                        }

                        // Moves crosshair
                        if (isLeftOn)   { offsetX--; }
                        if (isRightOn)  { offsetX++; }
                        if (isUpOn)     { offsetY--; }
                        if (isDownOn)   { offsetY++; }
                    }

                    // Sleeps with the refresh rate
                    Thread.Sleep(REFRESH_RATE);
                }
            }).Start();
        }

        private void Overlay_Load(object sender, EventArgs e) {
            // Makes the form transparent
            this.BackColor = Color.Wheat;
            this.TransparencyKey = Color.Wheat;
            // Removes the border
            this.FormBorderStyle = FormBorderStyle.None;

            // Makes the form on top of everything
            this.TopLevel = true;
            this.TopMost = true;

            // Makes the form click thru
            int initialStyle = Externs.GetWindowLong(this.Handle, -20);
            Externs.SetWindowLong(this.Handle, -20, initialStyle | 0x80000 | 0x20);

            // Starts the positioning thread  
            new Task(Prepaint).Start();
        }

        public void Prepaint() {
            while (true) {
                this.PositionWindow();

                // Force the ui to refresh
                this.SafeExec(form => form.Refresh());

                Thread.Sleep(REFRESH_RATE); // Refresh rate => 50ms
            }
        }

        private void PositionWindow() {
            // Invoke if needed to make it thread safe
            if (this.InvokeRequired) {
                this.Invoke(delegate { PositionWindow(); });
            }
            else {
                // Main screen dimensions in pixels
                Rectangle rect = Screen.FromControl(this).Bounds;

                this.Size = new Size(rect.Right - rect.Left, rect.Bottom - rect.Top);

                this.Top = rect.Top;
                this.Left = rect.Left;

                this.TopMost = true;
                this.TopLevel = true;
            }
        }

        private void Overlay_Paint(object sender, PaintEventArgs e) {
            // Initializes Graphics
            g = e.Graphics;
            
            // Gets the loaded config or the default one if no config is loaded
            Config cfg = Config.ActiveConfig ?? defaultCfg;

            // WHITE LINES
            try {
                float width = cfg.CalculateWidth() + offsetX;
                float height = cfg.CalculateHeight() + offsetY;

                g.DrawLine(pen, width - multi, height, width + multi, height);
                g.DrawLine(pen, width, height - multi, width, height + multi);
            }
            catch (ConfigParseException err) {
                // If config parsing error, show message and exits application
                MessageBox.Show(err.Message, "Error", MessageBoxButtons.OK);
                this.Close();
            }
            
            // BLACK LINES
            // No need to try here, since it uses the same values as the white lines
            float widthBlack = cfg.CalculateWidth() + 1 + offsetX;      // added a 1px offset
            float heightBlack = cfg.CalculateHeight() + 1 + offsetY;    // added a 1px offset

            g.DrawLine(penBlack, widthBlack - multi, heightBlack, widthBlack + multi, heightBlack);
            g.DrawLine(penBlack, widthBlack, heightBlack - multi, widthBlack, heightBlack + multi);
        }
    }

    internal static class ControlExt {
        /// <summary>
        /// Execute a thread safe function to a control
        /// </summary>
        /// <typeparam name="T">Control to execute</typeparam>
        /// <param name="control">Control to process</param>
        /// <param name="cb">Callback with the control as arguments</param>
        public static void SafeExec<T>(this T control, Action<T> cb) where T : Control {
            if (control.InvokeRequired) {
                control.Invoke(delegate { SafeExec(control, cb); });
            }
            else {
                cb(control);
            }
        }
    }
}
