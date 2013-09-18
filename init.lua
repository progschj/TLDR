font8x8 = image.load("fonts/terminal8x8_gs_ro_modified_alpha.png")
font8x12 = image.load("fonts/terminal8x12_gs_ro_modified_alpha.png")

require("scripts/serialize")

width = 60
height = 60

math.randomseed(os.time())

map_panel = panel.create(width, height, font8x8)
entities_panel = panel.create(width, height, font8x8)
info_panel = panel.create(20, 40, font8x12)
window:resize(640, 480)

dofile("rules.lua")

close = false

window:set_close_callback(
function(button)
    close = true
end)

window:set_key_callback(
function(key, scancode, action)
    if action == window.input.PRESS or action == window.input.REPEAT then
        if key == window.input.KEY_ESCAPE then
            close = true
        end
    end
end)

player = {}
player.x = 10
player.y = 10
player.symbol = string.byte('@')
player.color = 0xFFFF00
initActor(player)


g = grid.create(0, 0, width, height)
g:fill(0, 0, width, height, 0)

draw = function() 
    start = time.seconds()

    map_panel:fill_rect(0,0, width, height, string.byte(" "), 0x000000, 0x303030)
    map_panel:fill_indexed(g, {
        {string.byte('#'), 0x00AF00, 0x000000},
    })
    entities_panel:fill_rect(0,0, width, height, string.byte(" "), 0x000000, 0x000000)
    entities_panel:set(player.x, player.y, player.symbol, player.color, 0x000000)

    map_panel:draw(0, 0, 0.75, 1)
    entities_panel:blend_draw(0, 0, 0.75, 1)
    
    info_panel:fill_rect(0,0, 20, 40, string.byte(" "), 0x000000, 0xFFFDD0)
    info_panel:draw(0.75, 0, 0.25, 1)
    
    window:swap_buffers()

    stop = time.seconds()
    print((stop-start)*1e3)
end

while not close do
    window:wait_events()
    draw()
end
