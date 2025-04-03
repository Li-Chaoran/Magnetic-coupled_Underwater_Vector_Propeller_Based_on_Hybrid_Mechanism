using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class Gan1_value_show : MonoBehaviour
{
    public Text gan1_value;
    public GameObject qudonggan1;
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        string formatted = (qudonggan1.transform.eulerAngles.z-90.0f).ToString("F1");
        gan1_value.text = "孪生体控制值:" + formatted + "度";
    }
}
