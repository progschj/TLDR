width = 80
height = 60
font = image.load("fonts/terminal8x12_gs_ro_modified.png")
fontwidth, fontheight = font:size()
tilewidth = fontwidth/16
tileheight = fontheight/16

map = panel.create(width, height, font)
window:resize(map:pixelsize())

walls = grid.create(0, 0, width, height)
light = grid.create(0, 0, width, height)
distances = grid.create(0, 0, width, height)

for x=0,width-1 do
    for y=0,height-1 do
        local a = noise.perlin(x*0.05+100, y*0.05+200, 0.5)
        local b = noise.perlin(x*0.05-300, y*0.05+100, 0.5)
        local h = math.min(math.abs(a), math.abs(b))
        walls:set(x, y, math.max(0, 1-math.floor(10*h)))
    end
end

destination = {5, 5}
path.dijkstra(distances, walls, {destination}, 8)

close = false
mx = 0
my = 0

window:set_mouse_move_callback(
function(x, y)
    mx = x/tilewidth
    my = y/tileheight
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


while not close do
    window:wait_events()
    light:fill(0)
    fov.shadowcast(walls, light, mx, my, 30, 0.1)

    for x=0,width-1 do
        for y=0,height-1 do
           map:set(x, y, string.byte(" "), 0xFFFFFF, 0x010101*math.floor(255*math.min(light:get(x, y) + 0.1*walls:get(x, y), 1)))
        end
    end

    x = math.floor(mx)
    y = math.floor(my)

    map:set(x, y, string.byte("#"), 0xFF5F5F)
    while x ~= destination[1] or y ~= destination[2] do
        dx, dy = path.gradient(distances, x, y, 8)
        if dx == nil then break end
        x = x + dx
        y = y + dy
        map:set(x, y, string.byte("#"), 0xFF5F5F)
    end

    map:draw()
    window:swap_buffers()
end
