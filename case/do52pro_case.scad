// DO52 Pro — parametric case (tray + PCB spacer frame + switch plate)
// -------------------------------------------------------------------
// Reverse-engineered from photos on an A4 cutting mat (1 cm grid),
// calibrated against the controller's 2.54 mm header pitch.
// Geometry is drawn for the RIGHT half seen from the top (controller and
// thumb cluster on the left/inner side). Set side="left" to mirror it.
//
// Coordinates: origin = centre of the top key of the inner column (col 0),
// X to the right, Y up (away from the user), Z up from the tray bottom.
//
// Usage:
//   openscad -D 'part="plate"' -o plate.stl do52pro_case.scad
//   part = "plate" | "frame" | "tray" | "assembly" | "exploded" | "drawing"
// -------------------------------------------------------------------

/* [Output] */
part = "assembly";     // [plate, frame, tray, assembly, exploded, drawing]
side = "right";        // [right, left]

/* [Key grid] */
pitch      = 19.05;    // key pitch, both axes (measured 19.0–19.2)
// column stagger in mm (Y offset of each column's top key), col 0 = inner
stagger    = [0, 3.4, 8.0, 3.2, -2.0, -7.1];
rows       = [4, 4, 4, 4, 4, 4];   // regular rows per column
thumb_gap  = 21.3;     // row 4 -> thumb key, cols 0 and 1 (wider than pitch)
thumb_gap5 = 19.3;     // row 4 -> bottom key, col 5
// rotated key below col 4: [dx from col 4, gap below row 4, angle]
tilt_key   = [-2.0, 19.85, -8.5];
edge_margin = 2.0;     // plate edge beyond the key-unit squares

/* [Switch plate] */
plate_t     = 1.5;     // 1.5 = MX clip thickness
switch_hole = 14.0;    // MX cutout; add ~0.1–0.2 if your printer runs tight
plate_pocket_depth = 0.75;  // shallow recess beside the controller (0 = off)

/* [Spacer frame (between plate and PCB)] */
mx_plate_to_pcb = 5.0; // MX spec: plate top to PCB top
frame_hole  = 15.6;    // clearance around switch bottom housings
frame_t     = mx_plate_to_pcb - plate_t;

/* [Tray] */
pcb_t       = 1.6;
floor_t     = 2.0;
boss_h      = 3.0;     // PCB standoff height (clears hot-swap sockets)
boss_d      = 6.0;
boss_hole   = 1.8;     // M2 self-tapping pilot; 3.2 for M2 heat-set inserts
wall        = 3.0;
clearance   = 0.5;     // gap between PCB/plate edge and tray wall
wall_above_pcb = 0.0;  // raise the wall past the PCB top surface
ledge_w     = 1.0;     // inner ledge under the PCB edge (0 = off)
corner_r    = 1.0;     // outer corner rounding of the tray
feet        = [[-24, -2], [97, -5], [-24, -82], [97, -84]];
foot_d      = 10.0;
foot_depth  = 1.0;

/* [Holes] */
screw_d   = 2.4;       // clearance through plate and frame (M2)
// screw/standoff positions (also the tray bosses)
screws = [[-26.25, -27.8], [-13.98, -28.0], [0, -pitch/2],
          [5*19.05, -16.6], [96.6, -73.85]];
// small round holes in plate and frame: [x, y, d]
aux_holes = [[9.0, -38.6, 3.5], [9.0, -47.3, 3.5], [9.0, -57.9, 3.0],
             [-19.25, -56.5, 5.0], [-19.25, -65.7, 6.0]];
// large keyhole opening beside the thumb keys: [x, y, d, slot_w, slot_end_y]
keyhole = [-20.2, -77.5, 14.5, 2.5, -85.5];

/* [Controller side (inner edge)] */
inner_x      = -31.4;  // outer edge of the controller strip
rail_w       = 2.2;    // thin rail along the controller window
rail_top     = 13.15;
window_bot   = -25.05; // bottom of the open controller window
arm_bot      = -31.15; // bottom of the bar that carries two screws
slot_bot     = -41.0;  // bottom of the open slot below that bar
bottom_left_y = -91.1;
pocket       = [[-30.3, -50.7], [-11.1, -41.0]];  // plate recess corners

/* [Bottom edge (measured, smoothed)] */
bottom_curve = [[84.67, 0], [80.84, -93.81], [77.97, -90.72], [68.39, -88.39],
                [58.81, -87.43], [49.23, -87.23], [39.66, -87.81],
                [30.08, -88.39], [20.50, -88.78], [11.40, -89.36]];
bottom_step_x = 11.4;

/* [Exploded view] */
explode = 18;

$fn = 48;

// ---------------------------------------------------------------- derived
p  = pitch;
m  = edge_margin;
function ctop(c)  = stagger[c] + p/2 + m;
function right_x() = 5*p + p/2 + m;
function thumb5_y() = stagger[5] - 3*p - thumb_gap5;
function bottom_right_y() = thumb5_y() - p/2 - m;

keys = concat(
  [for (c = [0:5]) for (r = [0:rows[c]-1]) [c*p, stagger[c] - r*p, 0]],
  [[0, stagger[0] - 3*p - thumb_gap, 0],
   [p, stagger[1] - 3*p - thumb_gap, 0],
   [5*p, thumb5_y(), 0],
   [4*p + tilt_key[0], stagger[4] - 3*p - tilt_key[1], tilt_key[2]]]);

// top edge: stepped, follows the column stagger
function step_x(c) = stagger[c+1] > stagger[c] ? (c+1)*p - p/2 - m
                                               : c*p + p/2 + m;
top_edge = concat(
  [[-p/2, ctop(0)]],
  [for (c = [0:4]) each [[step_x(c), ctop(c)], [step_x(c), ctop(c+1)]]],
  [[right_x(), ctop(5)]]);

function chaikin(pts, n) = n == 0 ? pts :
  chaikin(concat([pts[0]],
                 [for (i = [0:len(pts)-2]) each
                   [pts[i]*0.75 + pts[i+1]*0.25, pts[i]*0.25 + pts[i+1]*0.75]],
                 [pts[len(pts)-1]]), n - 1);

curve_pts = [for (i = [0:len(bottom_curve)-1])
               i == 0 ? [bottom_curve[0][0], bottom_right_y()] : bottom_curve[i]];
bottom_edge = concat([[right_x(), bottom_right_y()]], chaikin(curve_pts, 3),
                     [[bottom_step_x, bottom_left_y]]);

// plate/frame outline: open controller window, slot and rail
inner_open = [
  [inner_x, bottom_left_y], [inner_x, slot_bot], [-p/2, slot_bot],
  [-p/2, arm_bot], [inner_x + 0.4, arm_bot], [inner_x + 0.4, rail_top],
  [inner_x + 0.4 + rail_w, rail_top], [inner_x + 0.4 + rail_w, window_bot],
  [-p/2, window_bot]];
// tray outline: same, with the controller side closed
inner_closed = [[inner_x, bottom_left_y], [inner_x, rail_top], [-p/2, rail_top]];

module outline_open()   polygon(concat(top_edge, bottom_edge, inner_open));
module outline_closed() polygon(concat(top_edge, bottom_edge, inner_closed));

module round_holes(extra = 0) {
  for (h = aux_holes) translate([h[0], h[1]]) circle(d = h[2] + extra);
  translate([keyhole[0], keyhole[1]]) circle(d = keyhole[2] + extra);
  translate([keyhole[0] - keyhole[3]/2, keyhole[4]])
    square([keyhole[3], keyhole[1] - keyhole[4]]);
  for (s = screws) translate(s) circle(d = screw_d);
}

module key_holes(size) {
  for (k = keys) translate([k[0], k[1]]) rotate(k[2]) square(size, center = true);
}

// ------------------------------------------------------------------ parts
module plate() {
  difference() {
    linear_extrude(plate_t) difference() {
      outline_open();
      key_holes(switch_hole);
      round_holes();
    }
    if (plate_pocket_depth > 0)
      translate([pocket[0][0], pocket[0][1], plate_t - plate_pocket_depth])
        cube([pocket[1][0] - pocket[0][0], pocket[1][1] - pocket[0][1], plate_t]);
  }
}

module frame() {
  linear_extrude(frame_t) difference() {
    outline_open();
    key_holes(frame_hole);
    round_holes();
  }
}

module tray() {
  wall_top = floor_t + boss_h + pcb_t + wall_above_pcb;
  difference() {
    union() {
      linear_extrude(floor_t)
        offset(r = corner_r) offset(delta = clearance + wall - corner_r) outline_closed();
      translate([0, 0, floor_t]) linear_extrude(wall_top - floor_t) difference() {
        offset(r = corner_r) offset(delta = clearance + wall - corner_r) outline_closed();
        offset(delta = clearance) outline_closed();
      }
      if (ledge_w > 0)
        translate([0, 0, floor_t]) linear_extrude(boss_h) difference() {
          offset(delta = clearance + 0.01) outline_closed();
          offset(delta = -ledge_w) outline_closed();
        }
      for (s = screws) translate([s[0], s[1], floor_t - 0.01])
        cylinder(d = boss_d, h = boss_h + 0.01);
    }
    for (s = screws) translate([s[0], s[1], floor_t]) cylinder(d = boss_hole, h = boss_h + 1);
    for (f = feet) translate([f[0], f[1], -0.01]) cylinder(d = foot_d, h = foot_depth + 0.01);
  }
}

// --------------------------------------------------------------- layouts
z_pcb   = floor_t + boss_h;
z_frame = z_pcb + pcb_t;
z_plate = z_frame + frame_t;

module pcb_ghost() {
  color("darkgreen", 0.35) translate([0, 0, z_pcb])
    linear_extrude(pcb_t) outline_closed();
}

module stack(e = 0) {
  color("dimgray") tray();
  translate([0, 0, e]) pcb_ghost();
  color("slategray") translate([0, 0, z_frame + 2*e]) frame();
  color("black") translate([0, 0, z_plate + 3*e]) plate();
}

module sided() { if (side == "left") mirror([1, 0, 0]) children(); else children(); }

// ---------------------------------------------------- 2-D drawing sheet
module dim_h(x0, x1, y, label, s = 3) {
  translate([x0, y - 0.15]) square([x1 - x0, 0.3]);
  for (x = [x0, x1]) translate([x - 0.15, y - 2]) square([0.3, 4]);
  translate([(x0 + x1)/2, y + 1.5]) text(label, size = s, halign = "center");
}
module dim_v(y0, y1, x, label, s = 3) {
  translate([x - 0.15, y0]) square([0.3, y1 - y0]);
  for (y = [y0, y1]) translate([x - 2, y - 0.15]) square([4, 0.3]);
  translate([x - 1.5, (y0 + y1)/2]) rotate(90) text(label, size = s, halign = "center");
}
module outline_only(w = 0.35) difference() { children(); offset(delta = -w) children(); }

module drawing() {
  W = right_x() - inner_x;
  H = ctop(2) - bottom_right_y();
  T = clearance + wall;
  // top views
  translate([0, 0]) {
    outline_only() projection() plate();
    for (k = keys) translate([k[0], k[1]]) circle(d = 0.8);
    dim_h(inner_x, right_x(), ctop(2) + 8, str("plate ", r1(W), " mm"));
    dim_v(bottom_right_y(), ctop(2), right_x() + 8, str(r1(H), " mm"));
    dim_h(0, p, ctop(0) + 3, str("pitch ", p), 2.5);
    translate([inner_x, bottom_right_y() - 12]) text("PLATE - top", size = 5);
  }
  translate([W + 40, 0]) {
    outline_only() projection() frame();
    translate([inner_x, bottom_right_y() - 12]) text("FRAME - top", size = 5);
  }
  translate([2*(W + 40), 0]) {
    outline_only() projection() tray();
    outline_only() offset(delta = clearance) outline_closed();
    for (s = screws) translate(s) outline_only() circle(d = boss_d);
    dim_h(inner_x - T, right_x() + T, ctop(2) + T + 8, str("tray ", r1(W + 2*T), " mm"));
    translate([inner_x, bottom_right_y() - 12]) text("TRAY - top", size = 5);
  }
  // front (looking from -Y, Z up) and right side (looking from +X, Y to the right)
  fy = bottom_right_y() - 45;
  translate([0, fy]) {
    projection() rotate([-90, 0, 0]) stack();
    dim_v(0, z_plate + plate_t, right_x() + T + 6, str(r1(z_plate + plate_t), " mm"), 2.5);
    translate([inner_x, -12]) text("ASSEMBLY - front", size = 5);
  }
  translate([W + 40 + inner_x - bottom_right_y(), fy]) {
    projection() rotate([-90, 0, 0]) rotate([0, 0, -90]) stack();
    translate([bottom_right_y(), -12]) text("ASSEMBLY - right", size = 5);
  }
  translate([2*(W + 40), fy]) {
    projection() rotate([-90, 0, 0]) tray();
    dim_v(0, floor_t + boss_h + pcb_t + wall_above_pcb, right_x() + T + 6,
          str(r1(floor_t + boss_h + pcb_t + wall_above_pcb), " mm"), 2.5);
    translate([inner_x, -12]) text("TRAY - front", size = 5);
  }
}

function r1(v) = round(v * 10) / 10;

// ------------------------------------------------------------------ main
sided() {
  if (part == "plate")    plate();
  if (part == "frame")    frame();
  if (part == "tray")     tray();
  if (part == "assembly") stack(0);
  if (part == "exploded") stack(explode);
  if (part == "drawing")  drawing();
}
