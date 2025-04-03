using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class Jizuo_value_show : MonoBehaviour
{
    public Text Jizuo_value;
    public GameObject chuanlianliangan1;
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        string formatted = chuanlianliangan1.GetComponent<joint1_rotation>().dm3410_pose.ToString("F1");
        Jizuo_value.text = "孪生体控制值:" + formatted + "度";
    }
}
