#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable

const uint TILE_SIZE   = 16u;
const uint TILE_STRIDE = 256u;

const uint MAX_LIGHTS = 1024u;

uint FirstOfTile(uvec2 tile, uint tilesPerRow) {
	return (tile.y * tilesPerRow + tile.x) * TILE_STRIDE;
}
