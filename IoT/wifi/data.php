    <?php
    include ('connection.php');
    $sql_insert = "INSERT INTO temperature (temperature,humidite,luminosite) VALUES ('".$_GET["temperature"]."','".$_GET["humidite"]."','".$_GET["luminosite"]."')";
    if(mysqli_query($con,$sql_insert))
    {
    echo "Insertion reussie.";
    mysqli_close($con);
    }
    else
    {
    echo "Erreur : ".mysqli_error($con );
    }
    ?>
