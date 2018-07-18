<html>
<body>

<h1>ROCmod custom color values</h1>

<table>
<?php
$r = 0;
while ( $r <= 100 ) {
	echo "<table>\n";
	$g = 0;
	while ( $g <= 100 ) {
		$b = 0;
		while ( $b <= 100 ) {
			$c = "#".sprintf("%02X%02X%02X",($r*255/100),($g*255/100),($b*255/100));
			echo "<tr><td align=right>$r,$g,$b =</td><td bgcolor=$c>$r,$g,$b</td></tr>\n";
			$b += 5;
		}
		$g += 5;
	}
	$r += 5;
	echo "</table>\n";
}
?>
</table>

</body>
</html>

