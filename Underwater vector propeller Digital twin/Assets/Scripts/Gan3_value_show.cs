using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class Gan3_value_show : MonoBehaviour
{
    public Text gan3_value;
    public GameObject qudonggan3;
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        string formatted = (qudonggan3.transform.eulerAngles.z-210.0f).ToString("F1");
        gan3_value.text = "孪生体控制值:" + formatted + "度";
    }
}
