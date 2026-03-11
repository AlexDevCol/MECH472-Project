# DirectX window size for 6 ft × 6 ft battlefield

The simulation uses a **6 ft × 6 ft** battlefield (72 × 72 inches). All simulation units (robots, obstacles, positions) are in **inches**.

Set the DirectX window to match so 1 inch = 10 pixels:

- **Width:** 720  
- **Height:** 720  

If your DirectX_window project uses a `window_size.txt` file, set it to:

```
720
720
1
```

(third value = use border, 1 = yes)

If you use a different window size, update `WindowWidth` and `WindowHeight` in `global_data.h` so that `PixelsPerInch` stays consistent (e.g. 720/72 = 10).
