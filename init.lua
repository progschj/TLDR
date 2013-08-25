width = 160
height = 120
font = image.load("fonts/terminal8x8_gs_ro_modified.png")
fontwidth, fontheight = font:size()
tilewidth = fontwidth/16
tileheight = fontheight/16

map = panel.create(width, height, font)
window:resize(map:pixel_size())

walls = grid.create(0, 0, width, height)
light = grid.create(0, 0, width, height)

for x=0,width-1 do
    for y=0,height-1 do
        local a = noise.perlin(x*0.05+100, y*0.05+200, 0.5)
        local b = noise.perlin(x*0.05-300, y*0.05+100, 0.5)
        local h = math.min(math.abs(a), math.abs(b))
        walls:set(x, y, math.max(0, 1-math.floor(10*h)))
    end
end

close = false
mouse = {x = 0, y = 0}

window:set_mouse_move_callback(
function(x, y)
    mouse.x = x/tilewidth
    mouse.y = y/tileheight
end)

window:set_close_callback(
function(button)
    close = true
end)

window:set_key_callback(
function(key)
    if key == window.input.KEY_ESCAPE then
        close = true
    end
end)

light:fill(0.0)

while not close do
    window:poll_events()
    
    fov.shadowcast(walls, light, mouse.x, mouse.y, 40, 0.1, 0.01)
    light:add(0.1)    
    map:fill_indexed(walls, light, {
        {string.byte(" "), 0xFFFFFF, 0x000000},
        {string.byte(" "), 0xFFFFFF, 0xFFFFFF}
    });
    light:add(-0.1)
    light:mul(0.99)
    
    map:draw()
    window:swap_buffers()
end
