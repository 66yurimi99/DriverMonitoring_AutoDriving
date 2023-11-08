<?php

include('config.php'); // 데이터베이스 설정 파일

$conn = mysqli_connect($dbHost, $dbUser, $dbPass);
mysqli_select_db($conn, $dbName);

if (isset($_GET['sleep'])) 
{	// Set 요청 : db 업데이트 (sleep)
    $value = $_GET['sleep'];

    $query = "update flag set value=$value where flag='sleep'";
    if (mysqli_query($conn, $query)) 
    {
        echo $value;
    }
} 
else 
{	// Get 요청 : db 조회 (sleep)

    // 캐시 파일에 저장된 값 가져오기
    $cacheFileName = 'sleep_value_cache.txt';

    $cacheValid = false;
	// 파일이 존재하고 캐시 유효 시간이 아직 남아있을 때, 캐시 파일에 저장된 값을 출력
    if (file_exists($cacheFileName)) 
    {
        $cacheFileTime = filemtime($cacheFileName);
        if (time() - $cacheFileTime <= 60 * 30)
        {
            $cacheValid = true;
        }
    }

    if ($cacheValid) 
    {
        // 캐시 파일에서 값 가져와서 출력
        $cachedValue = file_get_contents($cacheFileName);
        echo $cachedValue;
    } 
    else 
    {
        // 캐시가 유효하지 않을 경우, DB에서 값을 조회하고 캐시 파일 업데이트
        $query = "select value from flag where flag='sleep'";
        $result = mysqli_query($conn, $query);

        if ($result) 
        {
            $row = mysqli_fetch_assoc($result);
            $sleepValue = $row['value'];

            // 캐시 파일 업데이트
            file_put_contents($cacheFileName, $sleepValue);

            echo $sleepValue;
        }
    }
}

mysqli_close($conn);
?>
