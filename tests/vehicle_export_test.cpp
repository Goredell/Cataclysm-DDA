#include <optional>
#include <vector>

#include "cata_catch.h"
#include "character.h"
#include "json_loader.h"
#include "map_helpers.h"
#include "vehicle.h"
#include "veh_type.h"

static const vproto_id vehicle_prototype_veh_export_test( "veh_export_test" );

static bool operator==( const vehicle_prototype::zone_def &l, const vehicle_prototype::zone_def &r )
{
    return l.filter == r.filter && l.name == r.name && l.pt == r.pt && l.zone_type == r.zone_type;
}

static bool operator==( const vehicle_prototype::part_def &l, const vehicle_prototype::part_def &r )
{
    return l.ammo_qty == r.ammo_qty && l.part == r.part && l.variant == r.variant &&
           l.with_ammo == r.with_ammo && l.ammo_types == r.ammo_types && l.ammo_qty == r.ammo_qty &&
           l.fuel == r.fuel && l.tools == r.tools;
}

static bool operator==( const vehicle_item_spawn &l, const vehicle_item_spawn &r )
{
    return l.pos == r.pos && l.chance == r.chance &&
           l.with_ammo == r.with_ammo && l.with_magazine == r.with_magazine && l.item_ids == r.item_ids &&
           // NOLINTNEXTLINE(misc-redundant-expression)
           l.variant_ids == r.variant_ids && l.item_groups == r.item_groups;
}

TEST_CASE( "export_vehicle_test" )
{
    clear_map();
    map &here = get_map();
    // Spawn the vehicle with fuel.
    vehicle *veh_ptr = get_map().add_vehicle( vehicle_prototype_veh_export_test, tripoint_bub_ms::zero,
                       0_degrees, -1, 0 );
    REQUIRE( veh_ptr != nullptr );

    // To ensure the zones get placed.
    veh_ptr->set_owner( get_player_character() );
    veh_ptr->place_zones( here );
    veh_ptr->refresh( &here );
    veh_ptr->refresh_zones();

    std::ostringstream os;
    JsonOut jsout( os );
    vehicle_prototype::save_vehicle_as_prototype( *veh_ptr, jsout );
    const static std::string expected_veh_prototype =
        R"({"//1":"Although this vehicle can be readily spawned in game without further tweaking,","//2":"this is auto-generated by program, so please check against it before proceeding.","//3":"Only simple top-level items can be exported by this function.  Do not rely on this for exporting comestibles, containers, etc.","id":"/TO_BE_REPLACED/","type":"vehicle","name":"/TO_BE_REPLACED/","blueprint":["t&"],"parts":[{"x":0,"y":0,"parts":["frame#vertical_T_left",{"part":"tank_medium","fuel":"diesel"},"diesel_engine_v6","alternator_car","turret_mount",{"part":"turret_m240","ammo":100,"ammo_types":"762_51","ammo_qty":[500,500]},"seat#swivel_chair_front"]},{"x":0,"y":1,"parts":["frame#vertical_T_right",{"part":"veh_tools_workshop","tools":["water_purifier","pot","pan"]}]}],"items":[{"x":0,"y":0,"chance":100,"items":["saw"]}],"zones":[{"x":0,"y":0,"type":"LOOT_GUNS"}]})";
    CHECK( os.str() == expected_veh_prototype );

    // To ensure the exported prototype can be readily loaded.
    JsonValue jv = json_loader::from_string( os.str() );
    JsonObject jo = jv.get_object();

    // To suppress flexbuffer [error_skipped_members] error.
    jo.get_string( "id" );
    jo.get_string( "type" );

    // To check the exported prototype has up-to-date format and is correct.
    vehicle_prototype vpr;
    vpr.load( jo, "" );
    CHECK( vpr.item_spawns == ( *vehicle_prototype_veh_export_test ).item_spawns );
    CHECK( vpr.zone_defs == ( *vehicle_prototype_veh_export_test ).zone_defs );
    CHECK( vpr.parts == ( *vehicle_prototype_veh_export_test ).parts );
}
