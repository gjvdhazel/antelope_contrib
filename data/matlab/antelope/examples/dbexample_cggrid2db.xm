display('Running dbexample_cggrid2db')

echo on

% Construct a contrived grid:

[X,Y] = meshgrid(-2:0.2:2,-3:0.3:3);
Z = exp( -X.^2 - Y.^2 );
cgg = cggrid( X, Y, Z )

% Save this to a fake database as though it belonged
% to an earthquake:

output_dir = ['/tmp/exampledir_' getenv('USER')];
unix( ['/bin/rm -rf ' output_dir] );
unix( ['mkdir ' output_dir] );

output_dbname = [output_dir '/newdb'];
fid = fopen( output_dbname, 'w' );
fprintf( fid, '#\nschema rt1.0:gme1.0\n' );
fclose( fid );

db=dbopen( output_dbname,'r+' );
db=dblookup( db,'','origin','','' );

orid = dbnextid( db, 'orid' );
db.record = dbaddv( db, 'lat', -116, ...
			'lon', 34, ...
			'depth', 0, ...
			'time', str2epoch( '12/31/2002' ), ...
			'orid', orid );

cggrid2db( cgg, db, 'dbexample_fake', 'testgrid', ...
	   '%Y/%j/%{gridname}_%{recipe}.%{qgridfmt}', ...
	   't4', 'g' );

clear( cgg );

% Extract the grid and plot it:

db = dblookup( db, '', 'qgrid', '', '' );
db.record = 0;
gridfile = dbfilename( db );
cgg2 = cggrid( gridfile );
[myx, myy, myz] = cggrid_getmesh(cgg2);
surf(myx,myy,myz)

% (Allow time for figure to come up when running in batch mode)
pause(0.5);

dbclose( db );

clear( cgg2 );

echo off