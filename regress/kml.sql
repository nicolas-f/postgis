-- Regression tests for KML producer
-- Written by: Eduin Carrillo <yecarrillo@cas.gov.co>
--             � 2006 Corporacion Autonoma Regional de Santander - CAS
-- http://postgis.refractions.net/pipermail/postgis-devel/2006-December/002376.html

-- SPATIAL INFO NO AVAILABLE
SELECT 'no_srid_01', AsKML(GeomFromEWKT('SRID=1021892;POINT(1000000 1000000)'));

--- EPSG 4326 : WGS 84
INSERT INTO "spatial_ref_sys" ("srid","auth_name","auth_srid","srtext","proj4text") VALUES (4326,'EPSG',4326,'GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.01745329251994328,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4326"]]','+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs ');
--- EPSG 1021892 : Bogota 1975 / Colombia Bogota zone (deprecated)
INSERT INTO "spatial_ref_sys" ("srid","auth_name","auth_srid","srtext","proj4text") VALUES (1021892,'EPSG',1021892,'PROJCS["Bogota 1975 / Colombia Bogota zone (deprecated)",GEOGCS["Bogota 1975",DATUM["Bogota_1975",SPHEROID["International 1924",6378388,297,AUTHORITY["EPSG","7022"]],TOWGS84[307,304,-318,0,0,0,0],AUTHORITY["EPSG","6218"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.01745329251994328,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4218"]],PROJECTION["Transverse_Mercator"],PARAMETER["latitude_of_origin",4.599047222222222],PARAMETER["central_meridian",-74.08091666666667],PARAMETER["scale_factor",1],PARAMETER["false_easting",1000000],PARAMETER["false_northing",1000000],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AUTHORITY["EPSG","21892"]]','+proj=tmerc +lat_0=4.599047222222222 +lon_0=-74.08091666666667 +k=1.000000 +x_0=1000000 +y_0=1000000 +ellps=intl +towgs84=307,304,-318,0,0,0,0 +units=m +no_defs ');

-- NO SRID PROVIDED
SELECT 'no_srid_02', AsKML(GeomFromEWKT('POINT(1 1)'));

-- UNSUPPORTED GEOMETRY TYPES
SELECT 'invalid_01', AsKML(GeomFromEWKT('SRID=4326;CIRCULARSTRING(-2 0,0 2,2 0,0 2,2 4)'));
SELECT 'invalid_02', AsKML(GeomFromEWKT('SRID=4326;COMPOUNDCURVE(CIRCULARSTRING(0 0,1 1,1 0),(1 0,0 1))'));
SELECT 'invalid_03', AsKML(GeomFromEWKT('SRID=4326;CURVEPOLYGON(CIRCULARSTRING(-2 0,-1 -1,0 0,1 -1,2 0,0 2,-2 0),(-1 0,0 0.5,1 0,0 1,-1 0))'));
SELECT 'invalid_04', AsKML(GeomFromEWKT('SRID=4326;MULTICURVE((5 5,3 5,3 3,0 3),CIRCULARSTRING(0 0,2 1,2 2))'));
SELECT 'invalid_05', AsKML(GeomFromEWKT('SRID=4326;MULTISURFACE(CURVEPOLYGON(CIRCULARSTRING(-2 0,-1 -1,0 0,1 -1,2 0,0 2,-2 0),(-1 0,0 0.5,1 0,0 1,-1 0)),((7 8,10 10,6 14,4 11,7 8)))'));

-- PARAMETERS
SELECT 'parameter_01', AsKML(GeomFromEWKT('SRID=4326;POINT(1.1111111111111111 1.1111111111111111)'), 0);
SELECT 'parameter_02', AsKML(GeomFromEWKT('SRID=4326;POINT(1.1111111111111111 1.1111111111111111)'), 16);
SELECT 'parameter_03', AsKML(3,GeomFromEWKT('SRID=4326;POINT(1.1111111111111111 1.1111111111111111)'), 15);

-- SIMPLE FEATURES
-- San Felipe de Barajas Fortresses - Cartagena, Colombia (Placemark) http://en.wikipedia.org/wiki/Cartagena%2C_Bol%C3%ADvar
SELECT 'point_01', AsKML(GeomFromEWKT('SRID=4326;POINT(-75.55217297757488 10.42033663453054)'), 3);
SELECT 'point_02', AsKML(GeomFromEWKT('SRID=4326;POINT(-75.55217297757488 10.42033663453054)'), 8);
-- Olaya Herrera Airport - Medellin, Colombia (Path) http://en.wikipedia.org/wiki/Olaya_Herrera_Airport
SELECT 'linestring_01', AsKML(GeomFromEWKT('SRID=4326;LINESTRING(-75.58845168747847 6.230811711917435, -75.59257646818483 6.209034252575331)'), 3);
SELECT 'linestring_02', AsKML(GeomFromEWKT('SRID=4326;LINESTRING(-75.58845168747847 6.230811711917435, -75.59257646818483 6.209034252575331)'), 8);
-- Unicentro Shopping Centre - Bogota, Colombia (Polygon) http://www.unicentrobogota.com/
SELECT 'polygon_01', AsKML(SnapToGrid(GeomFromEWKT('SRID=4326;POLYGON((-74.0423991077642 4.70128819450968, -74.04209925973704 4.700950993650923, -74.0420182951016 4.701011516462908, -74.04183483125468 4.700831448835688, -74.0414862905795 4.70114015046422, -74.04132847200927 4.70097698051241, -74.04101990886149 4.701244756502166, -74.0411777421693 4.701425554204853, -74.04087244656924 4.701697127180076, -74.040940643764 4.701773680406961, -74.04089952744008 4.702605660205299, -74.04082413388542 4.702677666010084, -74.04106565350325 4.702949831221687, -74.04087265096442 4.703121094787139, -74.04118884713594 4.7034683330851, -74.04138264686453 4.703308983914091, -74.04173931195382 4.703688847671272, -74.04192370456995 4.703528238994438, -74.0419535628069 4.70355486096117, -74.04230657961881 4.703239478084376, -74.04207492135191 4.702978242710751, -74.0421546959669 4.702907817351857, -74.04195032608129 4.70267512672329, -74.04209558069562 4.702533577124568, -74.04203505048247 4.702466795922, -74.04205318281811 4.702038077671765, -74.04212335562635 4.701974337580458, -74.04198589330784 4.701816901909086, -74.04222167646385 4.70160442413543, -74.04214901975594 4.701519995786457, -74.0423991077642 4.70128819450968))'), 0.000001), 3);
SELECT 'polygon_02', AsKML(GeomFromEWKT('SRID=4326;POLYGON((-74.0423991077642 4.70128819450968, -74.04209925973704 4.700950993650923, -74.0420182951016 4.701011516462908, -74.04183483125468 4.700831448835688, -74.0414862905795 4.70114015046422, -74.04132847200927 4.70097698051241, -74.04101990886149 4.701244756502166, -74.0411777421693 4.701425554204853, -74.04087244656924 4.701697127180076, -74.040940643764 4.701773680406961, -74.04089952744008 4.702605660205299, -74.04082413388542 4.702677666010084, -74.04106565350325 4.702949831221687, -74.04087265096442 4.703121094787139, -74.04118884713594 4.7034683330851, -74.04138264686453 4.703308983914091, -74.04173931195382 4.703688847671272, -74.04192370456995 4.703528238994438, -74.0419535628069 4.70355486096117, -74.04230657961881 4.703239478084376, -74.04207492135191 4.702978242710751, -74.0421546959669 4.702907817351857, -74.04195032608129 4.70267512672329, -74.04209558069562 4.702533577124568, -74.04203505048247 4.702466795922, -74.04205318281811 4.702038077671765, -74.04212335562635 4.701974337580458, -74.04198589330784 4.701816901909086, -74.04222167646385 4.70160442413543, -74.04214901975594 4.701519995786457, -74.0423991077642 4.70128819450968))'), 8);

-- MULTI FEATURES
-- Transmilenio mass-transit system Portal Stations - Bogota, Colombia (Placemarks) http://en.wikipedia.org/wiki/List_of_TransMilenio_Stations
SELECT 'multipoint_01', AsKML(GeomFromEWKT('SRID=4326;MULTIPOINT((-74.04603457594773 4.754687006656519),(-74.095833 4.746435),(-74.11037547492613 4.7098754227297),(-74.120148 4.533696))'), 3);
SELECT 'multipoint_02', AsKML(GeomFromEWKT('SRID=4326;MULTIPOINT((-74.04603457594773 4.754687006656519),(-74.095833 4.746435),(-74.11037547492613 4.7098754227297),(-74.120148 4.533696))'), 8);

-- REPROJECTED DATA
-- Sun Door Interchange - Bucaramanga, Colombia (Placemark)
SELECT 'projection_01', AsKML(GeomFromEWKT('SRID=1021892;POINT(1106465.31495947 1277689.13470039)'), 0);
-- Chicamocha, Suarez and Fonce rivers confluence - Santander, Colombia (Placemark)
SELECT 'projection_02', AsKML(GeomFromEWKT('SRID=1021892;POINT(1097247.52170185 1240255.74263751)'), 0);
-- National Astronomical Observatory of Colombia - Bogota, Colombia (Placemark)
SELECT 'projection_03', AsKML(GeomFromEWKT('SRID=1021892;POINT(1000000 1000000)'), 0);

-- Repeat all tests with the new function names.
-- NO SRID PROVIDED
SELECT 'no_srid_03', ST_AsKML(ST_GeomFromEWKT('POINT(1 1)'));

-- UNSUPPORTED GEOMETRY TYPES
SELECT 'invalid_06', ST_AsKML(ST_GeomFromEWKT('SRID=4326;CIRCULARSTRING(-2 0,0 2,2 0,0 2,2 4)'));
SELECT 'invalid_07', ST_AsKML(ST_GeomFromEWKT('SRID=4326;COMPOUNDCURVE(CIRCULARSTRING(0 0,1 1,1 0),(1 0,0 1))'));
SELECT 'invalid_08', ST_AsKML(ST_GeomFromEWKT('SRID=4326;CURVEPOLYGON(CIRCULARSTRING(-2 0,-1 -1,0 0,1 -1,2 0,0 2,-2 0),(-1 0,0 0.5,1 0,0 1,-1 0))'));
SELECT 'invalid_09', ST_AsKML(ST_GeomFromEWKT('SRID=4326;MULTICURVE((5 5,3 5,3 3,0 3),CIRCULARSTRING(0 0,2 1,2 2))'));
SELECT 'invalid_10', ST_AsKML(ST_GeomFromEWKT('SRID=4326;MULTISURFACE(CURVEPOLYGON(CIRCULARSTRING(-2 0,-1 -1,0 0,1 -1,2 0,0 2,-2 0),(-1 0,0 0.5,1 0,0 1,-1 0)),((7 8,10 10,6 14,4 11,7 8)))'));

-- PARAMETERS
SELECT 'parameter_04', ST_AsKML(ST_GeomFromEWKT('SRID=4326;POINT(1.1111111111111111 1.1111111111111111)'), 0);
SELECT 'parameter_05', ST_AsKML(ST_GeomFromEWKT('SRID=4326;POINT(1.1111111111111111 1.1111111111111111)'), 16);
SELECT 'parameter_06', ST_AsKML(3,ST_GeomFromEWKT('SRID=4326;POINT(1.1111111111111111 1.1111111111111111)'), 15);

-- SIMPLE FEATURES
-- San Felipe de Barajas Fortresses - Cartagena, Colombia (Placemark) http://en.wikipedia.org/wiki/Cartagena%2C_Bol%C3%ADvar
SELECT 'point_03', ST_AsKML(ST_GeomFromEWKT('SRID=4326;POINT(-75.55217297757488 10.42033663453054)'));
SELECT 'point_04', ST_AsKML(ST_GeomFromEWKT('SRID=4326;POINT(-75.55217297757488 10.42033663453054)'), 8);
-- Olaya Herrera Airport - Medellin, Colombia (Path) http://en.wikipedia.org/wiki/Olaya_Herrera_Airport
SELECT 'linestring_03', ST_AsKML(ST_GeomFromEWKT('SRID=4326;LINESTRING(-75.58845168747847 6.230811711917435, -75.59257646818483 6.209034252575331)'), 3);
SELECT 'linestring_04', ST_AsKML(ST_GeomFromEWKT('SRID=4326;LINESTRING(-75.58845168747847 6.230811711917435, -75.59257646818483 6.209034252575331)'), 8);
-- Unicentro Shopping Centre - Bogota, Colombia (Polygon) http://www.unicentrobogota.com/
SELECT 'polygon_03', ST_AsKML(ST_SnapToGrid(GeomFromEWKT('SRID=4326;POLYGON((-74.0423991077642 4.70128819450968, -74.04209925973704 4.700950993650923, -74.0420182951016 4.701011516462908, -74.04183483125468 4.700831448835688, -74.0414862905795 4.70114015046422, -74.04132847200927 4.70097698051241, -74.04101990886149 4.701244756502166, -74.0411777421693 4.701425554204853, -74.04087244656924 4.701697127180076, -74.040940643764 4.701773680406961, -74.04089952744008 4.702605660205299, -74.04082413388542 4.702677666010084, -74.04106565350325 4.702949831221687, -74.04087265096442 4.703121094787139, -74.04118884713594 4.7034683330851, -74.04138264686453 4.703308983914091, -74.04173931195382 4.703688847671272, -74.04192370456995 4.703528238994438, -74.0419535628069 4.70355486096117, -74.04230657961881 4.703239478084376, -74.04207492135191 4.702978242710751, -74.0421546959669 4.702907817351857, -74.04195032608129 4.70267512672329, -74.04209558069562 4.702533577124568, -74.04203505048247 4.702466795922, -74.04205318281811 4.702038077671765, -74.04212335562635 4.701974337580458, -74.04198589330784 4.701816901909086, -74.04222167646385 4.70160442413543, -74.04214901975594 4.701519995786457, -74.0423991077642 4.70128819450968))'), 0.000001), 3);
SELECT 'polygon_04', ST_AsKML(ST_GeomFromEWKT('SRID=4326;POLYGON((-74.0423991077642 4.70128819450968, -74.04209925973704 4.700950993650923, -74.0420182951016 4.701011516462908, -74.04183483125468 4.700831448835688, -74.0414862905795 4.70114015046422, -74.04132847200927 4.70097698051241, -74.04101990886149 4.701244756502166, -74.0411777421693 4.701425554204853, -74.04087244656924 4.701697127180076, -74.040940643764 4.701773680406961, -74.04089952744008 4.702605660205299, -74.04082413388542 4.702677666010084, -74.04106565350325 4.702949831221687, -74.04087265096442 4.703121094787139, -74.04118884713594 4.7034683330851, -74.04138264686453 4.703308983914091, -74.04173931195382 4.703688847671272, -74.04192370456995 4.703528238994438, -74.0419535628069 4.70355486096117, -74.04230657961881 4.703239478084376, -74.04207492135191 4.702978242710751, -74.0421546959669 4.702907817351857, -74.04195032608129 4.70267512672329, -74.04209558069562 4.702533577124568, -74.04203505048247 4.702466795922, -74.04205318281811 4.702038077671765, -74.04212335562635 4.701974337580458, -74.04198589330784 4.701816901909086, -74.04222167646385 4.70160442413543, -74.04214901975594 4.701519995786457, -74.0423991077642 4.70128819450968))'), 8);

-- MULTI FEATURES
-- Transmilenio mass-transit system Portal Stations - Bogota, Colombia (Placemarks) http://en.wikipedia.org/wiki/List_of_TransMilenio_Stations
SELECT 'multipoint_03', ST_AsKML(ST_GeomFromEWKT('SRID=4326;MULTIPOINT((-74.04603457594773 4.754687006656519),(-74.095833 4.746435),(-74.11037547492613 4.7098754227297),(-74.120148 4.533696))'), 3);
SELECT 'multipoint_04', ST_AsKML(ST_GeomFromEWKT('SRID=4326;MULTIPOINT((-74.04603457594773 4.754687006656519),(-74.095833 4.746435),(-74.11037547492613 4.7098754227297),(-74.120148 4.533696))'), 8);

-- REPROJECTED DATA
-- Sun Door Interchange - Bucaramanga, Colombia (Placemark)
SELECT 'projection_04', ST_AsKML(ST_GeomFromEWKT('SRID=1021892;POINT(1106465.31495947 1277689.13470039)'), 3);
-- Chicamocha, Suarez and Fonce rivers confluence - Santander, Colombia (Placemark)
SELECT 'projection_05', ST_AsKML(ST_GeomFromEWKT('SRID=1021892;POINT(1097247.52170185 1240255.74263751)'), 8);
-- National Astronomical Observatory of Colombia - Bogota, Colombia (Placemark)
SELECT 'projection_06', ST_AsKML(ST_GeomFromEWKT('SRID=1021892;POINT(1000000 1000000)'), 3);

DELETE FROM spatial_ref_sys WHERE srid = 4326;
DELETE FROM spatial_ref_sys WHERE srid >= 1000000;